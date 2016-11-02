
#ifndef OZZ_C_WRAPPER_SHADER
#define OZZ_C_WRAPPER_SHADER

#define OZZ_INCLUDE_PRIVATE_HEADER
#include "cwrapperRenderer.h"

//-----------------------------------------------------------------------------
#ifndef CAPI_NO_SHADER
class CWrapperShader
{
private:
  // Shader program
  GLuint program_;

  // Vertex and fragment shaders
  GLuint vertex_;
  GLuint fragment_;

  // Uniform locations, in the order they were requested.
  ozz::Vector<GLint>::Std uniforms_;

  // Varying locations, in the order they were requested.
  ozz::Vector<GLint>::Std attribs_;

public:
  //---------------------------------------------------------------------------
  CWrapperShader();

  //---------------------------------------------------------------------------
  virtual ~CWrapperShader();

  //---------------------------------------------------------------------------
  static CWrapperShader* Build();

  //---------------------------------------------------------------------------
  void Bind(const ozz::math::Float4x4& _model, const ozz::math::Float4x4& _view_proj,
            GLsizei _pos_stride, GLsizei _pos_offset,
            GLsizei _normal_stride, GLsizei _normal_offset,
            GLsizei _uv_stride, GLsizei _uv_offset);

  //---------------------------------------------------------------------------
  void Unbind();

  //---------------------------------------------------------------------------
  // Constructs a shader from _vertex and _fragment glsl sources.
  // Mutliple source files can be specified using the *count argument.
  bool BuildFromSource(int _vertex_count, const char** _vertex, int _fragment_count, const char** _fragment);

  //---------------------------------------------------------------------------
  // Request an attribute location and pushes it to the uniform stack.
  // The varying location is then accessible thought attrib().
  bool FindAttrib(const char* _semantic);

  //---------------------------------------------------------------------------
  // Request an uniform location and pushes it to the uniform stack.
  // The uniform location is then accessible thought uniform().
  bool BindUniform(const char* _semantic);

  //---------------------------------------------------------------------------
  void UnbindAttribs();
};
#endif // CAPI_NO_SHADER
#endif // OZZ_C_WRAPPER_SHADER
