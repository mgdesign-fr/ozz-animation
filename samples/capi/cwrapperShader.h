
#ifndef OZZ_C_WRAPPER_SHADER
#define OZZ_C_WRAPPER_SHADER

#define OZZ_INCLUDE_PRIVATE_HEADER
#include <framework/internal/shader.h>

//-----------------------------------------------------------------------------
class CWrapperShader : public ozz::sample::internal::Shader
{
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
};

#endif // OZZ_C_WRAPPER_SHADER
