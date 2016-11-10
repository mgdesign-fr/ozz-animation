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

//-----------------------------------------------------------------------------
void buildFrustumPlanes(Plane* const _result, const float* _viewProj)
{
  const float xw = _viewProj[3];
  const float yw = _viewProj[7];
  const float zw = _viewProj[11];
  const float ww = _viewProj[15];

  const float xz = _viewProj[2];
  const float yz = _viewProj[6];
  const float zz = _viewProj[10];
  const float wz = _viewProj[14];

  Plane& nearPlane = _result[0];
  nearPlane.normal[0] = xw - xz;
  nearPlane.normal[1] = yw - yz;
  nearPlane.normal[2] = zw - zz;
  nearPlane.distance = ww - wz;

  Plane& farPlane = _result[1];
  farPlane.normal[0] = xw + xz;
  farPlane.normal[1] = yw + yz;
  farPlane.normal[2] = zw + zz;
  farPlane.distance = ww + wz;

  const float xx = _viewProj[0];
  const float yx = _viewProj[4];
  const float zx = _viewProj[8];
  const float wx = _viewProj[12];

  Plane& leftPlane = _result[2];
  leftPlane.normal[0] = xw - xx;
  leftPlane.normal[1] = yw - yx;
  leftPlane.normal[2] = zw - zx;
  leftPlane.distance = ww - wx;

  Plane& rightPlane = _result[3];
  rightPlane.normal[0] = xw + xx;
  rightPlane.normal[1] = yw + yx;
  rightPlane.normal[2] = zw + zx;
  rightPlane.distance = ww + wx;

  const float xy = _viewProj[1];
  const float yy = _viewProj[5];
  const float zy = _viewProj[9];
  const float wy = _viewProj[13];

  Plane& topPlane = _result[4];
  topPlane.normal[0] = xw + xy;
  topPlane.normal[1] = yw + yy;
  topPlane.normal[2] = zw + zy;
  topPlane.distance = ww + wy;

  Plane& bottomPlane = _result[5];
  bottomPlane.normal[0] = xw - xy;
  bottomPlane.normal[1] = yw - yy;
  bottomPlane.normal[2] = zw - zy;
  bottomPlane.distance = ww - wy;

  Plane* plane = _result;
  for (uint32_t ii = 0; ii < 6; ++ii)
  {
    const float len = sqrt(plane->normal[0]*plane->normal[0] + plane->normal[1]*plane->normal[1] + plane->normal[2]*plane->normal[2]);
    const float invLen = 1.0f/len;
		plane->normal[0] = plane->normal[0] * invLen;
		plane->normal[1] = plane->normal[1] * invLen;
		plane->normal[2] = plane->normal[2] * invLen;
    plane->distance *= invLen;
    ++plane;
  }
}
