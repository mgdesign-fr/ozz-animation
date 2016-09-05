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

#include "framework/application.h"
#include "framework/imgui.h"
#include "framework/utils.h"

#include "cwrapper.h"

class LoadSampleApplication : public ozz::sample::Application
{
 public:
  LoadSampleApplication() : data(NULL)
  {
  }

 protected:
  virtual bool OnUpdate(float _dt)
  {
    // Updates current animation time.
    update(data, _dt);
    return true;
  }

  virtual bool OnDisplay(ozz::sample::Renderer* _renderer)
  {
    // Samples animation, transforms to model space and renders.
    render(data, _renderer);
    return true;
  }

  virtual bool OnInitialize()
  {
    data = initialize(&defaultConfiguration);
    return (data != NULL);
  }

  virtual void OnDestroy()
  {
    dispose(data);
    data = NULL;
  }

  virtual bool OnGui(ozz::sample::ImGui* _im_gui) 
  {
    (_im_gui);
    return true;
  }

  virtual void GetSceneBounds(ozz::math::Box* _bound) const
  {
    (_bound);
  }

 private:
    Data* data;
};

int main(int _argc, const char** _argv)
{
  const char* title = "Ozz-animation sample: Binary animation/skeleton loading";
  return LoadSampleApplication().Run(_argc, _argv, "1.0", title);
}
