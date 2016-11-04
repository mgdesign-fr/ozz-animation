#include "cwrapper.h"
#include "cwrapperUtils.h"
#include "cwrapperRenderer.h"

#include <ozz/base/log.h>
#include <ozz/base/maths/vec_float.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/memory/allocator.h>

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/skeleton.h>

#include <framework/mesh.h>
#include <framework/utils.h>

//-----------------------------------------------------------------------------
struct Entity
{
  ozz::math::Float4x4 transform;
  ozz::sample::PlaybackController controller;
  ozz::animation::SamplingCache* cache;
  ozz::Range<ozz::math::SoaTransform> locals;
  ozz::Range<ozz::math::Float4x4> models;
  ozz::Range<ozz::math::Float4x4> skinning_matrices;
  uint32_t skeletonId;
  uint32_t animationId;
  uint32_t meshId;
  uint32_t textureId;
};

//-----------------------------------------------------------------------------
// NOTE(jeff) Orders matters for memory alignement!
struct Data
{
  struct RendererData* rendererData;
  ozz::animation::Skeleton skeletons[CONFIG_MAX_SKELETONS];
  ozz::animation::Animation animations[CONFIG_MAX_ANIMATIONS];
  ozz::sample::Mesh meshs[CONFIG_MAX_MESHS];
  Entity entities[CONFIG_MAX_ENTITIES];
  uint32_t entitiesCount;
};

//-----------------------------------------------------------------------------
struct Data* initialize(struct Config* config)
{
  bool success = true;

  // NOTE(jeff) Force alignement
  // NOTE(jeff) Use ozz allocator rather than _aligned_malloc because loading functions (for mesh/skeleton/anim) destroy / recreates memory, and crash.
  struct Data* data = (Data*)ozz::memory::default_allocator()->Allocate(sizeof(Data), 16);
  
  // Init le pointeur "cache" à null pour éviter les crashs si problème à l'init.
  uint32_t entityId = 0;
  for(entityId = 0; entityId < CONFIG_MAX_ENTITIES; ++entityId)
  {
    struct Entity& entity = data->entities[entityId];
    entity.cache = NULL;
  }

  // Renderer GL
  data->rendererData = rendererInitialize();
  
  // NB, en plus de servir de compteurs, ces variables stockent aussi le nombre total de ces éléments une fois chargés.
  uint32_t skeletonId = 0;
  uint32_t animationId = 0;
  uint32_t meshId = 0;
  uint32_t textureId = 0;

  // Reading skeletons.
  for(skeletonId = 0; skeletonId < CONFIG_MAX_SKELETONS; ++skeletonId)
  {
    char* skeletonPath = config->skeletonPaths[skeletonId];
    if(skeletonPath == NULL)
      break;

    success &= ozz::sample::LoadSkeleton(skeletonPath, &data->skeletons[skeletonId]);
  }

  if(success)
  {
    // Reading animations.
    for(animationId = 0; animationId < CONFIG_MAX_ANIMATIONS; ++animationId)
    {
      char* animationPath = config->animationPaths[animationId];
      if(animationPath == NULL)
        break;

      success &= ozz::sample::LoadAnimation(animationPath, &data->animations[animationId]);
    }
  }

  if(success)
  {
    // Reading meshs.
    for(meshId = 0; meshId < CONFIG_MAX_MESHS; ++meshId)
    {
      char* meshPath = config->meshsPaths[meshId];
      if(meshPath == NULL)
        break;

      success &= ozz::sample::LoadMesh(meshPath, &data->meshs[meshId]);
    }
  }

  if(success)
  {
    // Reading textures.
    for(textureId = 0; textureId < CONFIG_MAX_TEXTURES; ++textureId)
    {
      char* texturePath = config->texturesPaths[textureId];
      if(texturePath == NULL)
        break;

      success &= rendererLoadTexture(data->rendererData, texturePath, textureId);
    }
  }

  if(success)
  {
    // Building entities
    assert(config->entitiesCount <= CONFIG_MAX_ENTITIES);
    ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
    for(entityId = 0; entityId < config->entitiesCount; ++entityId)
    {
      struct Entity& entity = data->entities[entityId];
      struct EntityConfig& entityConfig = config->entities[entityId];

      assert(entityConfig.skeletonId < skeletonId);
      assert(entityConfig.animationId < animationId);
      assert(entityConfig.meshId < meshId);
      assert(entityConfig.textureId < textureId);

      entity.skeletonId = entityConfig.skeletonId;
      entity.animationId = entityConfig.animationId;
      entity.meshId = entityConfig.meshId;
      entity.textureId = entityConfig.textureId;

      const int num_joints = data->skeletons[entity.skeletonId].num_joints();
      const int num_soa_joints = data->skeletons[entity.skeletonId].num_soa_joints();

      // The number of joints of the mesh needs to match skeleton.
      if (data->meshs[entity.meshId].num_joints() != num_joints)
      {
        ozz::log::Err() << "The provided mesh doesn't match skeleton (joint count mismatch)." << std::endl;
        success = false;
      }

      // Allocates runtime buffers.
      entity.locals = allocator->AllocateRange<ozz::math::SoaTransform>(num_soa_joints);
      entity.models = allocator->AllocateRange<ozz::math::Float4x4>(num_joints);
      entity.skinning_matrices = allocator-> AllocateRange<ozz::math::Float4x4>(num_joints);

      // Allocates a cache that matches animation requirements.
      entity.cache = allocator->New<ozz::animation::SamplingCache>(num_joints);

      // Entity world transform
      floatPtrToOzzMatrix(entityConfig.transform, entity.transform);

      // Entity animation offset.
      entity.controller.set_time(entityConfig.timeOffset);
    }
    data->entitiesCount = config->entitiesCount;
  }

  // Clear memory on initialisation error.
  if(!success)
  {
    dispose(data);
    data = NULL;
  }

  return data;
}

//-----------------------------------------------------------------------------
void dispose(struct Data* data)
{
  ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
  for(uint32_t entityId = 0; entityId < CONFIG_MAX_ENTITIES; ++entityId)
  {
    struct Entity& entity = data->entities[entityId];
    if(entity.cache != NULL)
    {
      // Si entity cache est null, c'est que l'entité n'a pas été initialisée.
      allocator->Deallocate(entity.locals);
      allocator->Deallocate(entity.models);
      allocator->Deallocate(entity.skinning_matrices);
      allocator->Delete(entity.cache);
    }
  }

  if(data->rendererData != NULL)
  {
    // NB: textures are deallocated by rendererDispose function.
    rendererDispose(data->rendererData);
    data->rendererData = NULL;
  }

  ozz::memory::default_allocator()->Deallocate(data);
}

//-----------------------------------------------------------------------------
void setEntityTransform(struct Data* data, unsigned int entityId, float* transform)
{
  assert(entityId < data->entitiesCount);
  struct Entity& entity = data->entities[entityId];
  floatPtrToOzzMatrix(transform, entity.transform);
}

//-----------------------------------------------------------------------------
void update(struct Data* data, float _dt)
{
  for(uint32_t entityId = 0; entityId < data->entitiesCount; ++entityId)
  {
    struct Entity& entity = data->entities[entityId];
    ozz::animation::Skeleton& entitySkeleton = data->skeletons[entity.skeletonId];
    ozz::animation::Animation& entityAnimation = data->animations[entity.animationId];
    ozz::sample::Mesh& entityMesh = data->meshs[entity.meshId];
  
    // Updates current animation time.
    entity.controller.Update(entityAnimation, _dt);

    // Samples optimized animation at t = animation_time_.
    ozz::animation::SamplingJob sampling_job;
    sampling_job.animation = &entityAnimation;
    sampling_job.cache = entity.cache;
    sampling_job.time = entity.controller.time();
    sampling_job.output = entity.locals;
    bool success = sampling_job.Run();
    assert(success);

    // Converts from local space to model space matrices.
    ozz::animation::LocalToModelJob ltm_job;
    ltm_job.skeleton = &entitySkeleton;
    ltm_job.input = entity.locals;
    ltm_job.output = entity.models;
    success = ltm_job.Run();
    assert(success);

    // Update skinning matrices latest blending stage output.
    assert(entity.models.Count() == entity.skinning_matrices.Count() &&
           entity.models.Count() == entityMesh.inverse_bind_poses.size());

    // Builds skinning matrices, based on the output of the animation stage.
    for (size_t i = 0; i < entity.models.Count(); ++i)
      entity.skinning_matrices[i] = entity.models[i] * entityMesh.inverse_bind_poses[i];
  }
}

//-----------------------------------------------------------------------------
#if CAPI_NO_SHADER
void render(struct Data* data, float* viewProjMatrix, int position_attrib, int normal_attrib, int uv_attrib, int u_model_matrix, int u_view_projection_matrix, int u_texture)
#else
void render(struct Data* data, float* viewProjMatrix)
#endif
{
  // Setup view & proj matrix
  ozz::math::Float4x4 viewProj;
  floatPtrToOzzMatrix(viewProjMatrix, viewProj);

  // Render meshs
  for(uint32_t entityId = 0; entityId < data->entitiesCount; ++entityId)
  {
    struct Entity& entity = data->entities[entityId];
    ozz::sample::Mesh& entityMesh = data->meshs[entity.meshId];
#if CAPI_NO_SHADER
    rendererDrawSkinnedMesh(data->rendererData, viewProj, entityMesh, entity.textureId, entity.skinning_matrices, entity.transform, (GLint)position_attrib, (GLint)normal_attrib, (GLint)uv_attrib, (GLint)u_model_matrix, (GLint)u_view_projection_matrix, (GLint)u_texture);
#else
    rendererDrawSkinnedMesh(data->rendererData, viewProj, entityMesh, entity.textureId, entity.skinning_matrices, entity.transform);
#endif
  }
}
