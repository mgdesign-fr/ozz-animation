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


struct Data
{
  // Playback animation controller. This is a utility class that helps with
  // controlling animation playback time.
  ozz::sample::PlaybackController controller_;

  // Runtime skeleton.
  ozz::animation::Skeleton skeleton_;

  // Runtime animation.
  ozz::animation::Animation animation_;
  
  // The mesh used by the sample.
  ozz::sample::Mesh mesh_;

  // Sampling cache.
  ozz::animation::SamplingCache* cache_;

  // Buffer of local transforms as sampled from animation_.
  ozz::Range<ozz::math::SoaTransform> locals_;

  // Buffer of model space matrices.
  ozz::Range<ozz::math::Float4x4> models_;
  
  // Buffer of skinning matrices, result of the joint multiplication of the
  // inverse bind pose with the model space matrix.
  ozz::Range<ozz::math::Float4x4> skinning_matrices_;
};

Data* initialize()
{
  Data* data = new Data();
  data->cache_ = NULL;

  // Reading skeleton.
  if (!ozz::sample::LoadSkeleton("media/alain_skeleton.ozz", &data->skeleton_)) {
      return false;
  }

  // Reading animation.
  if (!ozz::sample::LoadAnimation("media/alain_atlas.ozz", &data->animation_)) {
      return false;
  }
    
  const int num_joints = data->skeleton_.num_joints();

  // Reading skinned mesh.
  if (!ozz::sample::LoadMesh("media/arnaud_mesh.ozz", &data->mesh_)) {
    return false;
  }

  // The number of joints of the mesh needs to match skeleton.
  if (data->mesh_.num_joints() != num_joints) {
    ozz::log::Err() << "The provided mesh doesn't match skeleton "
      "(joint count mismatch)." << std::endl;
    return false;
  }

  // Allocates runtime buffers.
  const int num_soa_joints = data->skeleton_.num_soa_joints();
  ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
  data->locals_ = allocator->AllocateRange<ozz::math::SoaTransform>(num_soa_joints);
  data->models_ = allocator->AllocateRange<ozz::math::Float4x4>(num_joints);
  data->skinning_matrices_ = allocator-> AllocateRange<ozz::math::Float4x4>(num_joints);

  // Allocates a cache that matches animation requirements.
  data->cache_ = allocator->New<ozz::animation::SamplingCache>(num_joints);
  return data;
}

void dispose(Data* data)
{
    ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
    allocator->Deallocate(data->locals_);
    allocator->Deallocate(data->models_);
    allocator->Deallocate(data->skinning_matrices_);
    allocator->Delete(data->cache_);
    data->cache_ = NULL;
    delete(data);
}

bool update(Data* data, float _dt)
{
  // Updates current animation time.
  data->controller_.Update(data->animation_, _dt);

  // Samples optimized animation at t = animation_time_.
  ozz::animation::SamplingJob sampling_job;
  sampling_job.animation = &data->animation_;
  sampling_job.cache = data->cache_;
  sampling_job.time = data->controller_.time();
  sampling_job.output = data->locals_;
  if (!sampling_job.Run()) {
    return false;
  }

  // Converts from local space to model space matrices.
  ozz::animation::LocalToModelJob ltm_job;
  ltm_job.skeleton = &data->skeleton_;
  ltm_job.input = data->locals_;
  ltm_job.output = data->models_;
  if (!ltm_job.Run()) {
    return false;
  }

  // Update skinning matrices latest blending stage output.
  assert(data->models_.Count() == data->skinning_matrices_.Count() &&
         data->models_.Count() == data->mesh_.inverse_bind_poses.size());

  // Builds skinning matrices, based on the output of the animation stage.
  for (size_t i = 0; i < data->models_.Count(); ++i) {
    data->skinning_matrices_[i] = data->models_[i] * data->mesh_.inverse_bind_poses[i];
  }

  return true;
}

bool render(Data* data, void* _renderer)
{
  ozz::sample::Renderer* rendererInstance = (ozz::sample::Renderer*)_renderer;
  return rendererInstance->DrawSkinnedMesh(data->mesh_,
                                           data->skinning_matrices_,
                                           ozz::math::Float4x4::identity());
}
