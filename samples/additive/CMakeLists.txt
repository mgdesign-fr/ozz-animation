
add_custom_command(
  DEPENDS $<$<BOOL:${ozz_build_fbx}>:BUILD_DATA>
          $<$<BOOL:${ozz_build_fbx}>:BUILD_DATA_SAMPLE>
          "${CMAKE_CURRENT_LIST_DIR}/README"
          "${ozz_media_directory}/bin/arnaud_mesh.ozz"
          "${ozz_media_directory}/bin/alain_skeleton.ozz"
          "${ozz_media_directory}/bin/alain_walk.ozz"
          "${ozz_media_directory}/bin/alain_crackhead_additive.ozz"
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/README"
          "${CMAKE_CURRENT_BINARY_DIR}/media/arnaud_mesh.ozz"
          "${CMAKE_CURRENT_BINARY_DIR}/media/alain_skeleton.ozz"
          "${CMAKE_CURRENT_BINARY_DIR}/media/alain_walk.ozz"
          "${CMAKE_CURRENT_BINARY_DIR}/media/alain_crackhead_additive.ozz"
  COMMAND ${CMAKE_COMMAND} -E make_directory media
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/README" .
  COMMAND ${CMAKE_COMMAND} -E copy "${ozz_media_directory}/bin/arnaud_mesh.ozz" "./media"
  COMMAND ${CMAKE_COMMAND} -E copy "${ozz_media_directory}/bin/alain_skeleton.ozz" "./media"
  COMMAND ${CMAKE_COMMAND} -E copy "${ozz_media_directory}/bin/alain_walk.ozz" "./media"
  COMMAND ${CMAKE_COMMAND} -E copy "${ozz_media_directory}/bin/alain_crackhead_additive.ozz" "./media")

add_executable(sample_additive
  sample_additive.cc
  "${CMAKE_CURRENT_BINARY_DIR}/README"
  "${CMAKE_CURRENT_BINARY_DIR}/media/arnaud_mesh.ozz"
  "${CMAKE_CURRENT_BINARY_DIR}/media/alain_skeleton.ozz"
  "${CMAKE_CURRENT_BINARY_DIR}/media/alain_walk.ozz"
  "${CMAKE_CURRENT_BINARY_DIR}/media/alain_crackhead_additive.ozz")
target_link_libraries(sample_additive
  sample_framework)

set_target_properties(sample_additive
  PROPERTIES FOLDER "samples")

if(EMSCRIPTEN)
  # Resource files are embedded to the output file with emscripten
  set_target_properties(sample_additive
    PROPERTIES LINK_FLAGS "--embed-file media --embed-file README --memory-init-file 0")

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/sample_additive.html
    ${CMAKE_CURRENT_BINARY_DIR}/sample_additive.js
    DESTINATION bin/samples/additive)
else()
  install(TARGETS sample_additive DESTINATION bin/samples/additive)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/media DESTINATION bin/samples/additive)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/README DESTINATION bin/samples/additive)
endif(EMSCRIPTEN)

add_test(NAME sample_additive COMMAND sample_additive "--max_idle_loops=${ozz_sample_testing_loops}" $<$<BOOL:${ozz_run_tests_headless}>:--norender>)
add_test(NAME sample_additive_path COMMAND sample_additive "--skeleton=media/alain_skeleton.ozz" "--animation=media/alain_walk.ozz" "--additive_animation=media/alain_crackhead_additive.ozz" "--mesh=media/arnaud_mesh.ozz" "--max_idle_loops=${ozz_sample_testing_loops}" $<$<BOOL:${ozz_run_tests_headless}>:--norender>)
add_test(NAME sample_additive_invalid_skeleton_path COMMAND sample_additive "--skeleton=media/bad_skeleton.ozz" $<$<BOOL:${ozz_run_tests_headless}>:--norender>)
set_tests_properties(sample_additive_invalid_skeleton_path PROPERTIES WILL_FAIL true)
add_test(NAME sample_additive_invalid_animation_path1 COMMAND sample_additive "--animation=media/bad_animation.ozz" $<$<BOOL:${ozz_run_tests_headless}>:--norender>)
set_tests_properties(sample_additive_invalid_animation_path1 PROPERTIES WILL_FAIL true)
add_test(NAME sample_additive_invalid_animation_path2 COMMAND sample_additive "--additive_animation=media/bad_animation.ozz" $<$<BOOL:${ozz_run_tests_headless}>:--norender>)
set_tests_properties(sample_additive_invalid_animation_path2 PROPERTIES WILL_FAIL true)
add_test(NAME sample_additive_invalid_mesh_path COMMAND sample_additive "--mesh=media/bad_mesh.ozz" $<$<BOOL:${ozz_run_tests_headless}>:--norender>)
set_tests_properties(sample_additive_invalid_mesh_path PROPERTIES WILL_FAIL true)
