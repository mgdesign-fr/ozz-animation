
add_library(ozz-capi SHARED 
  cwrapper.h
  cwrapper.cc
  cwrapperUtils.h
  cwrapperUtils.cc
  stb_image.h
  cwrapperRenderer.h
  cwrapperRenderer.cc
  cwrapperShader.h
  cwrapperShader.cc
  ../samples/framework/mesh.h
  ../samples/framework/mesh.cc
  ../samples/framework/utils.h
  ../samples/framework/utils.cc)
  
include_directories(${CMAKE_SOURCE_DIR}/samples)

add_definitions(-DCAPI_BUILD_DLL -DCAPI_NO_SHADER)

# CWrapper requires OpenGL package.
include_directories(${CMAKE_SOURCE_DIR}/extern/glfw/include) # Ce dossier contient les includes GL!
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

target_link_libraries(ozz-capi ozz_geometry
							   ozz_animation
							   ozz_options
							   ozz_base
							   ${OPENGL_LIBRARIES})
