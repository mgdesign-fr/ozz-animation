
#ifndef OZZ_C_WRAPPER_RENDERER
#define OZZ_C_WRAPPER_RENDERER

#include <framework/mesh.h>

//-----------------------------------------------------------------------------
struct RendererData;

//-----------------------------------------------------------------------------
struct RendererData* rendererInitialize();

//-----------------------------------------------------------------------------
void rendererDispose(struct RendererData* rendererData);

//-----------------------------------------------------------------------------
void rendererDrawSkinnedMesh(struct RendererData* rendererData, ozz::math::Float4x4& viewProjMatrix, const ozz::sample::Mesh& mesh, const unsigned int textureId, const ozz::Range<ozz::math::Float4x4> skinning_matrices, const ozz::math::Float4x4& transform);

//-----------------------------------------------------------------------------
bool rendererLoadTexture(struct RendererData* rendererData, const char* texturePath, unsigned int textureId);

//-----------------------------------------------------------------------------
void rendererUnloadTexture(struct RendererData* rendererData, unsigned int textureId);

#endif // OZZ_C_WRAPPER_RENDERER
