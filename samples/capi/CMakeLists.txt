
add_library(capi SHARED 
  cwrapper.h
  cwrapper.cc
  cwrapperUtils.h
  cwrapperUtils.cc
  stb_image.h
  cwrapperRenderer.h
  cwrapperRenderer.cc
  cwrapperShader.h
  cwrapperShader.cc)

add_definitions(-DCAPI_BUILD_DLL)
  
# CWrapper requires OpenGL package.
include_directories(${CMAKE_SOURCE_DIR}/extern/glfw/include)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

target_link_libraries(capi
  sample_framework)
