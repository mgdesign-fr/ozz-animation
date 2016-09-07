#include "cwrapperShader.h"

#include <ozz/base/maths/simd_math.h>

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
CWrapperShader::CWrapperShader()
{
}

//-----------------------------------------------------------------------------
CWrapperShader::~CWrapperShader()
{
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
void CWrapperShader::Bind(const ozz::math::Float4x4& _model, const ozz::math::Float4x4& _view_proj,
                          GLsizei _pos_stride, GLsizei _pos_offset,
                          GLsizei _normal_stride, GLsizei _normal_offset,
                          GLsizei _uv_stride, GLsizei _uv_offset)
{
  glUseProgram(program());

  const GLint position_attrib = attrib(0);
  glEnableVertexAttribArray(position_attrib);
  glVertexAttribPointer(position_attrib, 3, GL_FLOAT, GL_FALSE, _pos_stride, GL_PTR_OFFSET(_pos_offset));

  const GLint normal_attrib = attrib(1);
  glEnableVertexAttribArray(normal_attrib);
  glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_TRUE, _normal_stride, GL_PTR_OFFSET(_normal_offset));

  const GLint uv_attrib = attrib(2);
  glEnableVertexAttribArray(uv_attrib);
  glVertexAttribPointer(uv_attrib, 2, GL_FLOAT, GL_FALSE, _uv_stride, GL_PTR_OFFSET(_uv_offset));

  // Binds mw uniform
  float values[16];
  const GLint mw_uniform = uniform(0);
  ozz::math::StorePtrU(_model.cols[0], values + 0);
  ozz::math::StorePtrU(_model.cols[1], values + 4);
  ozz::math::StorePtrU(_model.cols[2], values + 8);
  ozz::math::StorePtrU(_model.cols[3], values + 12);
  glUniformMatrix4fv(mw_uniform, 1, false, values);

  // Binds mvp uniform
  const GLint mvp_uniform = uniform(1);
  ozz::math::StorePtrU(_view_proj.cols[0], values + 0);
  ozz::math::StorePtrU(_view_proj.cols[1], values + 4);
  ozz::math::StorePtrU(_view_proj.cols[2], values + 8);
  ozz::math::StorePtrU(_view_proj.cols[3], values + 12);
  glUniformMatrix4fv(mvp_uniform, 1, false, values);
}

//-----------------------------------------------------------------------------
void CWrapperShader::Unbind()
{
  UnbindAttribs();
  glUseProgram(0);
}
