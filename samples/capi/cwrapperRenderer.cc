#include "cwrapperRenderer.h"
#include "cwrapper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <Windows.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <ozz/base/maths/math_ex.h>
#include <ozz/geometry/runtime/skinning_job.h>

// Inclure après <Windows.h>
#include "cwrapperShader.h"

//-----------------------------------------------------------------------------
// Helper macro used to declare extension function pointer.
#define DECL_GL_EXT(_fct, _fct_type) _fct_type cwrapper_##_fct = NULL

//-----------------------------------------------------------------------------
#ifdef GL_VERSION_1_5
DECL_GL_EXT(glBindBuffer, PFNGLBINDBUFFERPROC);
DECL_GL_EXT(glDeleteBuffers, PFNGLDELETEBUFFERSPROC);
DECL_GL_EXT(glGenBuffers, PFNGLGENBUFFERSPROC);
DECL_GL_EXT(glBufferData, PFNGLBUFFERDATAPROC);
DECL_GL_EXT(glBufferSubData, PFNGLBUFFERSUBDATAPROC);
#else
#error "OpenGL 1_5 required"
#endif  // GL_VERSION_1_5

//-----------------------------------------------------------------------------
// Helper macro used to initialize extension function pointer.
#define INIT_GL_EXT(_fct, _fct_type) \
do {\
  cwrapper_##_fct = reinterpret_cast<_fct_type>(wglGetProcAddress(#_fct)); \
  assert (cwrapper_##_fct != NULL); \
} while (void(0), 0)

//-----------------------------------------------------------------------------
void _rendererInitializeOpenGLExtensions()
{
  INIT_GL_EXT(glBindBuffer, PFNGLBINDBUFFERPROC);
  INIT_GL_EXT(glDeleteBuffers, PFNGLDELETEBUFFERSPROC);
  INIT_GL_EXT(glGenBuffers, PFNGLGENBUFFERSPROC);
  INIT_GL_EXT(glBufferData, PFNGLBUFFERDATAPROC);
  INIT_GL_EXT(glBufferSubData, PFNGLBUFFERSUBDATAPROC);
}

//-----------------------------------------------------------------------------
// Provides helper macro to test for glGetError on a gl call.
#ifndef NDEBUG
#define CWRAPPER_GL(_f) do{\
  cwrapper_gl##_f;\
  GLenum error = glGetError();\
  assert(error == GL_NO_ERROR);\
} while(void(0), 0)
#else  // NDEBUG
#define CWRAPPER_GL(_f) cwrapper_gl##_f
#endif // NDEBUG

//-----------------------------------------------------------------------------
// Volatile memory buffer that can be used within function scope.
class ScratchBuffer
{
public:
  //---------------------------------------------------------------------------
  ScratchBuffer() : buffer_(NULL), size_(0)
  {
  };
  
  //---------------------------------------------------------------------------
  ~ScratchBuffer()
  {
    ozz::memory::default_allocator()->Deallocate(buffer_);
  };

  //---------------------------------------------------------------------------
  // Resizes the buffer to the new size and return the memory address.
  void* Resize(size_t _size)
  {
    if (_size > size_)
    {
      size_ = _size;
      buffer_ = ozz::memory::default_allocator()->Reallocate(buffer_, _size, 16);
    }
    return buffer_;
  }
private:
  void* buffer_;
  size_t size_;
};

//-----------------------------------------------------------------------------
struct RendererData
{
  GLuint glTextures[CONFIG_MAX_TEXTURES];
  GLuint dynamic_array_bo;
  GLuint dynamic_index_bo;
  ScratchBuffer scratch_buffer;
  CWrapperShader* shader;
};

//-----------------------------------------------------------------------------
RendererData* rendererInitialize()
{
  _rendererInitializeOpenGLExtensions();
  RendererData* rendererData = new RendererData();
  
  // Builds the dynamic vbo
  CWRAPPER_GL(GenBuffers(1, &rendererData->dynamic_array_bo));
  CWRAPPER_GL(GenBuffers(1, &rendererData->dynamic_index_bo));

  rendererData->shader = CWrapperShader::Build();
  assert(rendererData->shader != NULL);

  return rendererData;
}

//-----------------------------------------------------------------------------
void rendererDispose(RendererData* rendererData)
{
  if(rendererData->dynamic_array_bo)
  {
    CWRAPPER_GL(DeleteBuffers(1, &rendererData->dynamic_array_bo));
    rendererData->dynamic_array_bo = 0;
  }

  if(rendererData->dynamic_index_bo)
  {
    CWRAPPER_GL(DeleteBuffers(1, &rendererData->dynamic_index_bo));
    rendererData->dynamic_index_bo = 0;
  }
  
  ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
  allocator->Delete(rendererData->shader);
  rendererData->shader = NULL;

  // cleanup textures
  for(unsigned int textureId=0; textureId < CONFIG_MAX_TEXTURES; ++textureId)
    rendererUnloadTexture(rendererData, textureId);

  delete(rendererData);
  rendererData = NULL;
}

//-----------------------------------------------------------------------------
void rendererDrawSkinnedMesh(RendererData* rendererData, ozz::math::Float4x4& viewProjMatrix, const ozz::sample::Mesh& mesh, const unsigned int textureId, const ozz::Range<ozz::math::Float4x4> skinning_matrices, const ozz::math::Float4x4& transform)
{
  const int vertex_count = mesh.vertex_count();

  // Positions and normals are interleaved to improve caching while executing
  // skinning job.
  const GLsizei positions_offset = 0;
  const GLsizei normals_offset = sizeof(float) * 3;
  const GLsizei positions_stride = sizeof(float) * 6;
  const GLsizei normals_stride = positions_stride;
  const GLsizei skinned_data_size = vertex_count * positions_stride;

  // UVs are contiguous. They aren't transformed, so they can be directly
  // copied from source mesh which is non-interleaved as-well.
  const GLsizei uvs_offset = skinned_data_size;
  const GLsizei uvs_stride = sizeof(float) * 2;
  const GLsizei uvs_size = vertex_count * uvs_stride;

  // Reallocate vertex buffer.
  const GLsizei vbo_size = skinned_data_size + uvs_size;
  CWRAPPER_GL(BindBuffer(GL_ARRAY_BUFFER, rendererData->dynamic_array_bo));
  CWRAPPER_GL(BufferData(GL_ARRAY_BUFFER, vbo_size, NULL, GL_STREAM_DRAW));
  void* vbo_map = rendererData->scratch_buffer.Resize(vbo_size);

  // Iterate mesh parts and fills vbo.
  // Runs a skinning job per mesh part. Triangle indices are shared
  // across parts.
  size_t processed_vertex_count = 0;
  for (size_t i = 0; i < mesh.parts.size(); ++i)
  {
    const ozz::sample::Mesh::Part& part = mesh.parts[i];

    // Skip this iteration if no vertex.
    const size_t part_vertex_count = part.positions.size() / 3;
    if (part_vertex_count == 0)
    {
      continue;
    }

    // Fills the job.
    ozz::geometry::SkinningJob skinning_job;
    skinning_job.vertex_count = static_cast<int>(part_vertex_count);
    const int part_influences_count = part.influences_count();

    // Clamps joints influence count according to the option.
    skinning_job.influences_count = part_influences_count;

    // Setup skinning matrices, that came from the animation stage before being
    // multiplied by inverse model-space bind-pose.
    skinning_job.joint_matrices = skinning_matrices;

    // Setup joint's indices.
    skinning_job.joint_indices = make_range(part.joint_indices);
    skinning_job.joint_indices_stride = sizeof(uint16_t) * part_influences_count;

    // Setup joint's weights.
    if (part_influences_count > 1)
    {
      skinning_job.joint_weights = make_range(part.joint_weights);
      skinning_job.joint_weights_stride = sizeof(float) * (part_influences_count - 1);
    }

    // Setup input positions, coming from the loaded mesh.
    skinning_job.in_positions = make_range(part.positions);
    skinning_job.in_positions_stride = sizeof(float) * 3;

    // Setup output positions, coming from the rendering output mesh buffers.
    // We need to offset the buffer every loop.
    skinning_job.out_positions.begin = reinterpret_cast<float*>(ozz::PointerStride(vbo_map, positions_offset + processed_vertex_count * positions_stride));
    skinning_job.out_positions.end = ozz::PointerStride(skinning_job.out_positions.begin, part_vertex_count * positions_stride);
    skinning_job.out_positions_stride = positions_stride;

    // Setup normals if input are provided.
    float* out_normal_begin = reinterpret_cast<float*>(ozz::PointerStride(vbo_map, normals_offset + processed_vertex_count * normals_stride));
    const float* out_normal_end = ozz::PointerStride(out_normal_begin, part_vertex_count * normals_stride);

    if (part.normals.size() == part.positions.size())
    {
      // Setup input normals, coming from the loaded mesh.
      skinning_job.in_normals = make_range(part.normals);
      skinning_job.in_normals_stride = sizeof(float) * 3;

      // Setup output normals, coming from the rendering output mesh buffers.
      // We need to offset the buffer every loop.
      skinning_job.out_normals.begin = out_normal_begin;
      skinning_job.out_normals.end = out_normal_end;
      skinning_job.out_normals_stride = normals_stride;
    }
    else
    {
      // Fills output with default normals.
      for (float* normal = out_normal_begin; normal < out_normal_end; normal = ozz::PointerStride(normal, normals_stride))
      {
        normal[0] = 0.f;
        normal[1] = 1.f;
        normal[2] = 0.f;
      }
    }

    // Execute the job, which should succeed unless a parameter is invalid.
    assert(skinning_job.Run());

    // UVs
    assert(part_vertex_count == part.uvs.size() / 2);
    
    // Optimal path used when the right number of uvs is provided.
    memcpy(ozz::PointerStride(vbo_map, uvs_offset + processed_vertex_count * uvs_stride),
           array_begin(part.uvs),
           part_vertex_count * uvs_stride);
    
    // Some more vertices were processed.
    processed_vertex_count += part_vertex_count;
  }

  // Updates dynamic vertex buffer with skinned data.
  CWRAPPER_GL(BufferSubData(GL_ARRAY_BUFFER, 0, vbo_size, vbo_map));

  // Binds shader with this array buffer.
  rendererData->shader->Bind(transform,
                             viewProjMatrix,
                             positions_stride, positions_offset,
                             normals_stride, normals_offset,
                             uvs_stride, uvs_offset);

  CWRAPPER_GL(BindBuffer(GL_ARRAY_BUFFER, 0));

  // Maps the index dynamic buffer and update it.
  const ozz::sample::Mesh::TriangleIndices& indices = mesh.triangle_indices;
  CWRAPPER_GL(BindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererData->dynamic_index_bo));
  CWRAPPER_GL(BufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ozz::sample::Mesh::TriangleIndices::value_type), array_begin(indices), GL_STREAM_DRAW));
  
  // Binds texture
  glBindTexture(GL_TEXTURE_2D, rendererData->glTextures[textureId]);

  // Draws the mesh.
  assert(sizeof(ozz::sample::Mesh::TriangleIndices::value_type) == 2);
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_SHORT, 0);

  // Unbinds.
  glBindTexture(GL_TEXTURE_2D, 0);
  CWRAPPER_GL(BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
  rendererData->shader->Unbind();
}

//-----------------------------------------------------------------------------
bool rendererLoadTexture(RendererData* rendererData, const char* texturePath, unsigned int textureId)
{
  bool success = false;
  int imageWidth, imageHeight;
  int imageBpp = 3;

  // Open image
  unsigned char* imageData = stbi_load(texturePath, &imageWidth, &imageHeight, &imageBpp, 0);
  if(imageData != NULL)
  {
    // Create GL texture and load image data in it
    glGenTextures(1, &rendererData->glTextures[textureId]);
    glBindTexture(GL_TEXTURE_2D, rendererData->glTextures[textureId]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGB,
                 imageWidth, imageHeight,
                 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, 0);

    // All done free memory
    stbi_image_free(imageData);
    imageData = NULL;
    success = true;
  }

  return success;
}

//-----------------------------------------------------------------------------
void rendererUnloadTexture(RendererData* rendererData, unsigned int textureId)
{
  if(rendererData->glTextures[textureId] != 0)
  {
    glDeleteTextures(1, &rendererData->glTextures[textureId]);
    rendererData->glTextures[textureId] = 0;
  }
}
