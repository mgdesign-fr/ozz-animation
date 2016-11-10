
#ifndef OZZ_C_WRAPPER_UTILS 
#define OZZ_C_WRAPPER_UTILS

#include "cwrapper.h"
#include <ozz/base/maths/simd_math.h>

//-----------------------------------------------------------------------------
ozz::math::Float4x4&  floatPtrToOzzMatrix(const float* in, ozz::math::Float4x4& out);

//-----------------------------------------------------------------------------
float* ozzMatrixToFloatPtr(const ozz::math::Float4x4& in, float* out);

//-----------------------------------------------------------------------------
struct Plane
{
  float normal[3];
  float distance;
};

//-----------------------------------------------------------------------------
void buildFrustumPlanes(Plane* const _result, const float* _viewProj);

#endif // OZZ_C_WRAPPER_UTILS
