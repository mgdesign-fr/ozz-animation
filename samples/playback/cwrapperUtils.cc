#include "cwrapperUtils.h"

//-----------------------------------------------------------------------------
// ozz est column major convention comme rtgu (comme gl), donc on peut charger directement comme ça:
ozz::math::Float4x4& floatPtrToOzzMatrix(const float* in, ozz::math::Float4x4& out)
{
  out.cols[0] = ozz::math::simd_float4::LoadPtrU(&in[0]);
  out.cols[1] = ozz::math::simd_float4::LoadPtrU(&in[4]);
  out.cols[2] = ozz::math::simd_float4::LoadPtrU(&in[8]);
  out.cols[3] = ozz::math::simd_float4::LoadPtrU(&in[12]);
  return out;
}

//-----------------------------------------------------------------------------
float* ozzMatrixToFloatPtr(const ozz::math::Float4x4& in, float* out)
{
  ozz::math::StorePtrU(in.cols[0], &out[0]);
  ozz::math::StorePtrU(in.cols[1], &out[4]);
  ozz::math::StorePtrU(in.cols[2], &out[8]);
  ozz::math::StorePtrU(in.cols[3], &out[12]);
  return out;
}
