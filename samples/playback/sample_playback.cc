//----------------------------------------------------------------------------//
//                                                                            //
// ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation  //
// and distributed under the MIT License (MIT).                               //
//                                                                            //
// Copyright (c) 2015 Guillaume Blanc                                         //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// all copies or substantial portions of the Software.                        //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
//                                                                            //
//----------------------------------------------------------------------------//

#include "ozz/animation/runtime/skeleton.h"
#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/animation/runtime/local_to_model_job.h"

#include "ozz/base/log.h"

#include "ozz/base/memory/allocator.h"

#include "ozz/base/maths/vec_float.h"
#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/soa_transform.h"

#include "ozz/options/options.h"

#include "framework/application.h"
#include "framework/renderer.h"
#include "framework/imgui.h"
#include "framework/utils.h"
#include "framework/mesh.h"

// Skeleton archive can be specified as an option.
OZZ_OPTIONS_DECLARE_STRING(
  skeleton,
  "Path to the skeleton (ozz archive format).",
  "media/alain_skeleton.ozz",
  false)

// Animation archive can be specified as an option.
OZZ_OPTIONS_DECLARE_STRING(
  animation,
  "Path to the animation (ozz archive format).",
  "media/alain_atlas.ozz",
  false)

// Mesh archive can be specified as an option.
OZZ_OPTIONS_DECLARE_STRING(
  mesh,
  "Path to the skinned mesh (ozz archive format).",
  "media/arnaud_mesh.ozz",
  false)

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

bool initialize(Data* data)
{
  ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
  data->cache_ = NULL;

  // Reading skeleton.
  if (!ozz::sample::LoadSkeleton(OPTIONS_skeleton, &data->skeleton_)) {
      return false;
  }

  // Reading animation.
  if (!ozz::sample::LoadAnimation(OPTIONS_animation, &data->animation_)) {
      return false;
  }
    
  const int num_joints = data->skeleton_.num_joints();

  // Reading skinned mesh.
  if (!ozz::sample::LoadMesh(OPTIONS_mesh, &data->mesh_)) {
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
  data->locals_ = allocator->AllocateRange<ozz::math::SoaTransform>(num_soa_joints);
  data->models_ = allocator->AllocateRange<ozz::math::Float4x4>(num_joints);
  data->skinning_matrices_ = allocator-> AllocateRange<ozz::math::Float4x4>(num_joints);

  // Allocates a cache that matches animation requirements.
  data->cache_ = allocator->New<ozz::animation::SamplingCache>(num_joints);
  return true;
}

void dispose(Data* data)
{
    ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
    allocator->Deallocate(data->locals_);
    allocator->Deallocate(data->models_);
    allocator->Deallocate(data->skinning_matrices_);
    allocator->Delete(data->cache_);
    data->cache_ = NULL;
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

bool render(Data* data, ozz::sample::Renderer* _renderer)
{
    return _renderer->DrawSkinnedMesh(data->mesh_,
                                      data->skinning_matrices_,
                                      ozz::math::Float4x4::identity());
}

class LoadSampleApplication : public ozz::sample::Application {
 public:
  LoadSampleApplication()
  {
  }

 protected:
  // Updates current animation time.
  virtual bool OnUpdate(float _dt) {
    return update(&data, _dt);
  }

  // Samples animation, transforms to model space and renders.
  virtual bool OnDisplay(ozz::sample::Renderer* _renderer) {
    return render(&data, _renderer);
  }

  virtual bool OnInitialize() {
    return initialize(&data);
  }

  virtual void OnDestroy() {
    dispose(&data);
  }

  virtual bool OnGui(ozz::sample::ImGui* _im_gui) {
    // Exposes animation runtime playback controls.
    {
      static bool open = true;
      ozz::sample::ImGui::OpenClose oc(_im_gui, "Animation control", &open);
      if (open) {
        data.controller_.OnGui(data.animation_, _im_gui);
      }
    }
    return true;
  }

  virtual void GetSceneBounds(ozz::math::Box* _bound) const {
    ozz::sample::ComputePostureBounds(data.models_, _bound);
  }

 private:
    Data data;
};

int main(int _argc, const char** _argv) {
  const char* title =
    "Ozz-animation sample: Binary animation/skeleton loading";
  return LoadSampleApplication().Run(_argc, _argv, "1.0", title);
}
