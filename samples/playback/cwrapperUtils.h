
#ifndef OZZ_C_WRAPPER_UTILS 
#define OZZ_C_WRAPPER_UTILS

#include <ozz/base/maths/simd_math.h>

//-----------------------------------------------------------------------------
ozz::math::Float4x4&  floatPtrToOzzMatrix(const float* in, ozz::math::Float4x4& out);

//-----------------------------------------------------------------------------
float* ozzMatrixToFloatPtr(const ozz::math::Float4x4& in, float* out);

#endif // OZZ_C_WRAPPER_UTILS
