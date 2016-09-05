#include "cwrapper.h"

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
#include <framework/renderer.h>
#include <framework/utils.h>

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
};

struct Data
{
  ozz::animation::Skeleton skeletons[CONFIG_MAX_SKELETONS];
  ozz::animation::Animation animations[CONFIG_MAX_ANIMATIONS];
  ozz::sample::Mesh meshs[CONFIG_MAX_MESHS];
  Entity entities[CONFIG_MAX_ENTITIES];
  uint32_t entitiesCount;
};

Data* initialize(Config* config)
{
  bool success = true;
  Data* data = new Data();
  
  // NB, en plus de servir de compteurs, ces variables stockent aussi le nombre total de ces éléments une fois chargés.
  uint32_t skeletonId = 0;
  uint32_t animationId = 0;
  uint32_t meshId = 0;

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
    // Building entities
    assert(config->entitiesCount <= CONFIG_MAX_ENTITIES);
    uint32_t entityId = 0;
    ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
    for(entityId = 0; entityId < config->entitiesCount; ++entityId)
    {
      Entity& entity = data->entities[entityId];
      EntityConfig& entityConfig = config->entities[entityId];

      assert(entityConfig.skeletonId < skeletonId);
      assert(entityConfig.animationId < animationId);
      assert(entityConfig.meshId < meshId);

      entity.skeletonId = entityConfig.skeletonId;
      entity.animationId = entityConfig.animationId;
      entity.meshId = entityConfig.meshId;

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
      // ozz est column major convention comme rtgu (comme gl), donc on peut charger directement comme ça:
      entity.transform.cols[0] = ozz::math::simd_float4::LoadPtrU(&entityConfig.transform[0]);
      entity.transform.cols[1] = ozz::math::simd_float4::LoadPtrU(&entityConfig.transform[4]);
      entity.transform.cols[2] = ozz::math::simd_float4::LoadPtrU(&entityConfig.transform[8]);
      entity.transform.cols[3] = ozz::math::simd_float4::LoadPtrU(&entityConfig.transform[12]);
      //entity.transform = ozz::math::Float4x4::identity();
      
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

void dispose(Data* data)
{
  ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
  for(uint32_t entityId = 0; entityId < data->entitiesCount; ++entityId)
  {
    Entity& entity = data->entities[entityId];
    allocator->Deallocate(entity.locals);
    allocator->Deallocate(entity.models);
    allocator->Deallocate(entity.skinning_matrices);
    allocator->Delete(entity.cache);
  }
  delete(data);
}

void update(Data* data, float _dt)
{
  for(uint32_t entityId = 0; entityId < data->entitiesCount; ++entityId)
  {
    Entity& entity = data->entities[entityId];
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
    assert(sampling_job.Run());

    // Converts from local space to model space matrices.
    ozz::animation::LocalToModelJob ltm_job;
    ltm_job.skeleton = &entitySkeleton;
    ltm_job.input = entity.locals;
    ltm_job.output = entity.models;
    assert(ltm_job.Run());

    // Update skinning matrices latest blending stage output.
    assert(entity.models.Count() == entity.skinning_matrices.Count() &&
           entity.models.Count() == entityMesh.inverse_bind_poses.size());

    // Builds skinning matrices, based on the output of the animation stage.
    for (size_t i = 0; i < entity.models.Count(); ++i)
      entity.skinning_matrices[i] = entity.models[i] * entityMesh.inverse_bind_poses[i];
  }
}

void render(Data* data, void* _renderer)
{
  ozz::sample::Renderer* rendererInstance = (ozz::sample::Renderer*)_renderer;
  for(uint32_t entityId = 0; entityId < data->entitiesCount; ++entityId)
  {
    Entity& entity = data->entities[entityId];
    ozz::sample::Mesh& entityMesh = data->meshs[entity.meshId];

    rendererInstance->DrawSkinnedMesh(entityMesh,
                                      entity.skinning_matrices,
                                      entity.transform);
  }
}
