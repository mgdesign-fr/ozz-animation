
#ifndef OZZ_C_WRAPPER_RENDERER
#define OZZ_C_WRAPPER_RENDERER

#include <framework/mesh.h>

//-----------------------------------------------------------------------------
struct RendererData;

//-----------------------------------------------------------------------------
RendererData* rendererInitialize();

//-----------------------------------------------------------------------------
void rendererDispose(RendererData* rendererData);

//-----------------------------------------------------------------------------
void rendererDrawSkinnedMesh(RendererData* rendererData, ozz::math::Float4x4& viewProjMatrix, const ozz::sample::Mesh& mesh, const unsigned int textureId, const ozz::Range<ozz::math::Float4x4> skinning_matrices, const ozz::math::Float4x4& transform);

//-----------------------------------------------------------------------------
bool rendererLoadTexture(RendererData* rendererData, const char* texturePath, unsigned int textureId);

//-----------------------------------------------------------------------------
void rendererUnloadTexture(RendererData* rendererData, unsigned int textureId);

#endif // OZZ_C_WRAPPER_RENDERER
