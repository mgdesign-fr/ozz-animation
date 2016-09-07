
add_library(ozz-capi SHARED 
  cwrapper.h
  cwrapper.cc
  cwrapperUtils.h
  cwrapperUtils.cc
  stb_image.h
  cwrapperRenderer.h
  cwrapperRenderer.cc)
  
include_directories(${CMAKE_SOURCE_DIR}/samples)

add_definitions(-DCAPI_BUILD_DLL -DCAPI_NO_SHADER)
  
# CWrapper requires OpenGL package.
include_directories(${CMAKE_SOURCE_DIR}/extern/glfw/include)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

target_link_libraries(ozz-capi
  sample_framework)
