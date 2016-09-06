
add_custom_command(
  DEPENDS BUILD_DATA
          "${CMAKE_CURRENT_LIST_DIR}/README"
          "${ozz_media_directory}/bin/alain_skeleton.ozz"
          "${ozz_media_directory}/bin/alain_atlas.ozz"
          "${ozz_media_directory}/bin/arnaud_mesh.ozz"
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/README"
          "${CMAKE_CURRENT_BINARY_DIR}/media/alain_skeleton.ozz"
          "${CMAKE_CURRENT_BINARY_DIR}/media/alain_atlas.ozz"
          "${CMAKE_CURRENT_BINARY_DIR}/media/arnaud_mesh.ozz"
  COMMAND ${CMAKE_COMMAND} -E make_directory media
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/README" .
  COMMAND ${CMAKE_COMMAND} -E copy "${ozz_media_directory}/bin/alain_skeleton.ozz" "./media"
  COMMAND ${CMAKE_COMMAND} -E copy "${ozz_media_directory}/bin/alain_atlas.ozz" "./media"
  COMMAND ${CMAKE_COMMAND} -E copy "${ozz_media_directory}/bin/arnaud_mesh.ozz" "./media")

add_executable(sample_playback
  sample_playback.cc
  ../capi/cwrapper.h
  ../capi/cwrapper.cc
  ../capi/cwrapperUtils.h
  ../capi/cwrapperUtils.cc
  ../capi/stb_image.h
  ../capi/cwrapperRenderer.h
  ../capi/cwrapperRenderer.cc
  ../capi/cwrapperShader.h
  ../capi/cwrapperShader.cc
  "${CMAKE_CURRENT_BINARY_DIR}/README"
  "${CMAKE_CURRENT_BINARY_DIR}/media/alain_skeleton.ozz"
  "${CMAKE_CURRENT_BINARY_DIR}/media/alain_atlas.ozz"
  "${CMAKE_CURRENT_BINARY_DIR}/media/arnaud_mesh.ozz")
  
  
include_directories(../capi)
 
# CWrapper requires OpenGL package.
if(NOT EMSCRIPTEN)
  include_directories(${CMAKE_SOURCE_DIR}/extern/glfw/include)
  find_package(OpenGL REQUIRED)
  include_directories(${OPENGL_INCLUDE_DIR})
endif()
  
target_link_libraries(sample_playback
  sample_framework)

set_target_properties(sample_playback
  PROPERTIES FOLDER "samples")

if(EMSCRIPTEN)
  # Resource files are embedded to the output file with emscripten
  set_target_properties(sample_playback
    PROPERTIES LINK_FLAGS "--embed-file media --embed-file README --memory-init-file 0")

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/sample_playback.html
    ${CMAKE_CURRENT_BINARY_DIR}/sample_playback.js
    DESTINATION bin/samples/playback)
else()
  install(TARGETS sample_playback DESTINATION bin/samples/playback)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/media DESTINATION bin/samples/playback)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/README DESTINATION bin/samples/playback)
endif(EMSCRIPTEN)

add_test(NAME sample_playback COMMAND sample_playback "--max_idle_loops=${ozz_sample_testing_loops}" $<$<BOOL:${ozz_run_tests_headless}>:--norender>)
add_test(NAME sample_playback_path1 COMMAND sample_playback "--skeleton=media/alain_skeleton.ozz" "--animation=media/alain_atlas.ozz" "--max_idle_loops=${ozz_sample_testing_loops}" $<$<BOOL:${ozz_run_tests_headless}>:--norender>)
add_test(NAME sample_playback_invalid_skeleton_path COMMAND sample_playback "--skeleton=media/bad_skeleton.ozz" $<$<BOOL:${ozz_run_tests_headless}>:--norender>)
set_tests_properties(sample_playback_invalid_skeleton_path PROPERTIES WILL_FAIL true)
add_test(NAME sample_playback_invalid_animation_path COMMAND sample_playback "--animation=media/bad_animation.ozz" $<$<BOOL:${ozz_run_tests_headless}>:--norender>)
set_tests_properties(sample_playback_invalid_animation_path PROPERTIES WILL_FAIL true)

