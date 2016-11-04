#include "cwrapperShader.h"

#ifndef CAPI_NO_SHADER
#include <ozz/base/maths/simd_math.h>
#include "ozz/base/log.h"

//-----------------------------------------------------------------------------
// Helper macro used to declare extension function pointer.
#define SHADER_DECL_GL_EXT(_fct, _fct_type) extern _fct_type cwrapper_##_fct;

//-----------------------------------------------------------------------------
SHADER_DECL_GL_EXT(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC);
SHADER_DECL_GL_EXT(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC);
SHADER_DECL_GL_EXT(glUseProgram, PFNGLUSEPROGRAMPROC);
SHADER_DECL_GL_EXT(glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC);
SHADER_DECL_GL_EXT(glAttachShader, PFNGLATTACHSHADERPROC);
SHADER_DECL_GL_EXT(glCompileShader, PFNGLCOMPILESHADERPROC);
SHADER_DECL_GL_EXT(glCreateProgram, PFNGLCREATEPROGRAMPROC);
SHADER_DECL_GL_EXT(glCreateShader, PFNGLCREATESHADERPROC);
SHADER_DECL_GL_EXT(glDeleteShader, PFNGLDELETESHADERPROC);
SHADER_DECL_GL_EXT(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC);
SHADER_DECL_GL_EXT(glGetShaderSource, PFNGLGETSHADERSOURCEPROC);
SHADER_DECL_GL_EXT(glLinkProgram, PFNGLLINKPROGRAMPROC);
SHADER_DECL_GL_EXT(glShaderSource, PFNGLSHADERSOURCEPROC);
SHADER_DECL_GL_EXT(glGetShaderiv, PFNGLGETSHADERIVPROC);
SHADER_DECL_GL_EXT(glGetProgramiv, PFNGLGETPROGRAMIVPROC);
SHADER_DECL_GL_EXT(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC);
SHADER_DECL_GL_EXT(glGetAttribLocation, PFNGLGETATTRIBLOCATIONPROC);
SHADER_DECL_GL_EXT(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC);
SHADER_DECL_GL_EXT(glDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC);
SHADER_DECL_GL_EXT(glDetachShader, PFNGLDETACHSHADERPROC);
SHADER_DECL_GL_EXT(glDeleteProgram, PFNGLDELETEPROGRAMPROC);

//-----------------------------------------------------------------------------
const char* cwrapperShaderVS =
  "uniform mat4 u_mw;\n"
  "uniform mat4 u_mvp;\n"
  "attribute vec3 a_position;\n"
  "attribute vec3 a_normal;\n"
  "attribute vec2 a_uv;\n"
  "varying vec3 v_world_normal;\n"
  "varying vec2 v_vertex_uv;\n"
  "void main() {\n"
  "  mat4 world_matrix = u_mw;\n"
  "  vec4 vertex = vec4(a_position.xyz, 1.);\n"
  "  gl_Position = u_mvp * world_matrix * vertex;\n"
  "  mat3 cross_matrix = mat3(\n"
  "    cross(world_matrix[1].xyz, world_matrix[2].xyz),\n"
  "    cross(world_matrix[2].xyz, world_matrix[0].xyz),\n"
  "    cross(world_matrix[0].xyz, world_matrix[1].xyz));\n"
  "  float invdet = 1.0 / dot(cross_matrix[2], world_matrix[2].xyz);\n"
  "  mat3 normal_matrix = cross_matrix * invdet;\n"
  "  v_world_normal = normal_matrix * a_normal;\n"
  "  v_vertex_uv = a_uv;\n"
  "}\n";

//-----------------------------------------------------------------------------
const char* cwrapperShaderFS =
  "vec3 lerp(in vec3 alpha, in vec3 a, in vec3 b) {\n"
  "  return a + alpha * (b - a);\n"
  "}\n"
  "vec4 lerp(in vec4 alpha, in vec4 a, in vec4 b) {\n"
  "  return a + alpha * (b - a);\n"
  "}\n"
  "uniform sampler2D u_texture;\n"
  "varying vec3 v_world_normal;\n"
  "varying vec2 v_vertex_uv;\n"
  "void main() {\n"
  "  vec3 normal = normalize(v_world_normal);\n"
  "  vec3 alpha = (normal + 1.) * .5;\n"
  "  vec4 bt = lerp(\n"
  "    alpha.xzxz, vec4(.3, .3, .7, .7), vec4(.4, .4, .8, .8));\n"
  "  gl_FragColor = vec4(\n"
  "     lerp(alpha.yyy, vec3(bt.x, .3, bt.y), vec3(bt.z, .8, bt.w)), 1.);\n"
  "  gl_FragColor *= texture2D(u_texture, vec2(v_vertex_uv.x, -v_vertex_uv.y));\n"
  "}\n";

//-----------------------------------------------------------------------------
GLuint CompileShader(GLenum _type, int _count, const char** _src)
{
  GLuint shader = cwrapper_glCreateShader(_type);
  CWRAPPER_GL(ShaderSource(shader, _count, _src, NULL));
  CWRAPPER_GL(CompileShader(shader));

  int infolog_length = 0;
  CWRAPPER_GL(GetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolog_length));
  if (infolog_length > 1)
  {
    char* info_log = ozz::memory::default_allocator()->Allocate<char>(infolog_length);
    int chars_written  = 0;
    CWRAPPER_GL(GetShaderInfoLog(shader, infolog_length, &chars_written, info_log));
    ozz::memory::default_allocator()->Deallocate(info_log);
  }

  int status;
  CWRAPPER_GL(GetShaderiv(shader, GL_COMPILE_STATUS, &status));
  if (status)
    return shader;

  CWRAPPER_GL(DeleteShader(shader));
  return 0;
}

//-----------------------------------------------------------------------------
CWrapperShader::CWrapperShader()
{
    program_ = 0;
    vertex_ = 0;
    fragment_ = 0;
}

//-----------------------------------------------------------------------------
CWrapperShader::~CWrapperShader()
{
  if (vertex_)
  {
    CWRAPPER_GL(DetachShader(program_, vertex_));
    CWRAPPER_GL(DeleteShader(vertex_));
  }
  
  if (fragment_)
  {
    CWRAPPER_GL(DetachShader(program_, fragment_));
    CWRAPPER_GL(DeleteShader(fragment_));
  }

  if (program_)
  {
    CWRAPPER_GL(DeleteProgram(program_));
  }
}

//-----------------------------------------------------------------------------
bool CWrapperShader::BuildFromSource(int _vertex_count, const char** _vertex, int _fragment_count, const char** _fragment)
{
  // Tries to compile shaders.
  GLuint vertex_shader = 0;
  if (_vertex)
  {
    vertex_shader = CompileShader(GL_VERTEX_SHADER, _vertex_count, _vertex);
    if (!vertex_shader)
      return false;
  }

  GLuint fragment_shader = 0;
  if (_fragment)
  {
    fragment_shader = CompileShader(GL_FRAGMENT_SHADER, _fragment_count, _fragment);
    if (!fragment_shader)
    {
      if (vertex_shader)
        CWRAPPER_GL(DeleteShader(vertex_shader));

      return false;
    }
  }

  // Shaders are compiled, builds program.
  program_ = cwrapper_glCreateProgram();
  vertex_ = vertex_shader;
  fragment_ = fragment_shader;
  CWRAPPER_GL(AttachShader(program_, vertex_shader));
  CWRAPPER_GL(AttachShader(program_, fragment_shader));
  CWRAPPER_GL(LinkProgram(program_));

  int infolog_length = 0;
  CWRAPPER_GL(GetProgramiv(program_, GL_INFO_LOG_LENGTH, &infolog_length));
  if (infolog_length > 1)
  {
    char* info_log = ozz::memory::default_allocator()->Allocate<char>(infolog_length);
    int chars_written  = 0;
    CWRAPPER_GL(GetProgramInfoLog(program_, infolog_length, &chars_written, info_log));
    ozz::memory::default_allocator()->Deallocate(info_log);
  }

  return true;
}

//-----------------------------------------------------------------------------
CWrapperShader* CWrapperShader::Build()
{
  bool success = true;

  const char* vs[] = { cwrapperShaderVS };
  const char* fs[] = { cwrapperShaderFS };

  CWrapperShader* shader = ozz::memory::default_allocator()->New<CWrapperShader>();
  success &= shader->BuildFromSource(OZZ_ARRAY_SIZE(vs), vs,
                                     OZZ_ARRAY_SIZE(fs), fs);

  // Binds default attributes
  success &= shader->FindAttrib("a_position");
  success &= shader->FindAttrib("a_normal");
  success &= shader->FindAttrib("a_uv");

  // Binds default uniforms
  success &= shader->BindUniform("u_mw");
  success &= shader->BindUniform("u_mvp");

  if (!success)
  {
    ozz::memory::default_allocator()->Delete(shader);
    shader = NULL;
  }

  return shader;
}

//-----------------------------------------------------------------------------
bool CWrapperShader::FindAttrib(const char* _semantic)
{
  GLint location = cwrapper_glGetAttribLocation(program_, _semantic);
  if (location == -1)
    return false;
  
  attribs_.push_back(location);
  return true;
}

//-----------------------------------------------------------------------------
bool CWrapperShader::BindUniform(const char* _semantic) {
  GLint location = cwrapper_glGetUniformLocation(program_, _semantic);
  if (location == -1)
    return false;
  
  uniforms_.push_back(location);
  return true;
}

//-----------------------------------------------------------------------------
void CWrapperShader::Bind(const ozz::math::Float4x4& _model, const ozz::math::Float4x4& _view_proj,
                          GLsizei _pos_stride, GLsizei _pos_offset,
                          GLsizei _normal_stride, GLsizei _normal_offset,
                          GLsizei _uv_stride, GLsizei _uv_offset)
{
  CWRAPPER_GL(UseProgram(program_));

  const GLint position_attrib = attribs_[0];
  CWRAPPER_GL(EnableVertexAttribArray(position_attrib));
  CWRAPPER_GL(VertexAttribPointer(position_attrib, 3, GL_FLOAT, GL_FALSE, _pos_stride, GL_PTR_OFFSET(_pos_offset)));

  const GLint normal_attrib = attribs_[1];
  CWRAPPER_GL(EnableVertexAttribArray(normal_attrib));
  CWRAPPER_GL(VertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_TRUE, _normal_stride, GL_PTR_OFFSET(_normal_offset)));

  const GLint uv_attrib = attribs_[2];
  CWRAPPER_GL(EnableVertexAttribArray(uv_attrib));
  CWRAPPER_GL(VertexAttribPointer(uv_attrib, 2, GL_FLOAT, GL_FALSE, _uv_stride, GL_PTR_OFFSET(_uv_offset)));

  // Binds mw uniform
  float values[16];
  const GLint mw_uniform = uniforms_[0];
  ozz::math::StorePtrU(_model.cols[0], values + 0);
  ozz::math::StorePtrU(_model.cols[1], values + 4);
  ozz::math::StorePtrU(_model.cols[2], values + 8);
  ozz::math::StorePtrU(_model.cols[3], values + 12);
  CWRAPPER_GL(UniformMatrix4fv(mw_uniform, 1, false, values));

  // Binds mvp uniform
  const GLint mvp_uniform = uniforms_[1];
  ozz::math::StorePtrU(_view_proj.cols[0], values + 0);
  ozz::math::StorePtrU(_view_proj.cols[1], values + 4);
  ozz::math::StorePtrU(_view_proj.cols[2], values + 8);
  ozz::math::StorePtrU(_view_proj.cols[3], values + 12);
  CWRAPPER_GL(UniformMatrix4fv(mvp_uniform, 1, false, values));
}

//-----------------------------------------------------------------------------
void CWrapperShader::Unbind()
{
  UnbindAttribs();
  CWRAPPER_GL(UseProgram(0));
}

//-----------------------------------------------------------------------------
void CWrapperShader::UnbindAttribs()
{
  for (size_t i = 0; i < attribs_.size(); ++i)
    CWRAPPER_GL(DisableVertexAttribArray(attribs_[i]));
}

#endif // CAPI_NO_SHADER