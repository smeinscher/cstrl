cmake_minimum_required(VERSION 3.29)
project(cstrl C)
project(cstrl-test C)
project(cstrl-sandbox C)

set(CMAKE_C_STANDARD 11)

find_package(OpenGL REQUIRED)

add_library(log.c SHARED
        extern/log.c/log.c
        extern/log.c/log.h)
target_compile_options(log.c PRIVATE -DLOG_USE_COLOR)

add_library(cstrl SHARED
        extern/glad/glad.c
        extern/glad/glad.h
        include/cstrl/cstrl_defines.h
        include/cstrl/cstrl_math.h
        src/util/dynamic_array.c
        include/cstrl/cstrl_util.h
        include/cstrl/cstrl_types.h
        include/cstrl/cstrl_platform.h
        src/platform/platform_win32.c
        src/renderer/opengl/opengl_platform.h
        src/renderer/opengl/opengl_platform_win32.c
        src/renderer/opengl/opengl_renderer.c
        include/cstrl/cstrl_renderer.h
        src/platform/platform_internal.h
        src/platform/platform_linux.c
        src/renderer/opengl/opengl_platform_linux.c
        src/renderer/opengl/opengl_shader.c
        src/renderer/opengl/opengl_texture.c
        src/util/file_helpers.c
        extern/stb/stb_image.c
        extern/stb/stb_image.h
        src/renderer/glsl_shader_programs.h
        src/renderer/camera.c
        src/platform/platform_common.c
        src/util/general_helpers.c
        include/cstrl/cstrl_ui.h
        src/ui/ui.c
)

add_executable(cstrl-test
        test/main.c
        test/test_manager/test_manager.c
        test/test_manager/test_manager.h
        test/test_manager/test_types.h
        test/unit_tests/cstrl_math/test_vec2.c
        test/unit_tests/cstrl_math/test_vec2.h
        test/unit_tests/cstrl_math/test_vec3.c
        test/unit_tests/cstrl_math/test_vec3.h
        test/unit_tests/cstrl_math/test_vec4.c
        test/unit_tests/cstrl_math/test_vec4.h
        test/unit_tests/util/test_dynamic_array.c
        test/unit_tests/util/test_dynamic_array.h
        test/unit_tests/cstrl_math/test_quat.c
        test/unit_tests/cstrl_math/test_quat.h
        test/unit_tests/renderer/test_camera.c
        test/unit_tests/renderer/test_camera.h
        test/unit_tests/unit_tests.c
        test/unit_tests/unit_tests.h
        test/manual_tests/basic/test_4_cubes_scene.c
        test/manual_tests/basic/test_4_cubes_scene.h
        test/integration_tests/integration_tests.c
        test/integration_tests/integration_tests.h
        test/manual_tests/manual_tests.c
        test/manual_tests/manual_tests.h
)

add_executable(cstrl-sandbox
        sandbox/learnopengl/learnopengl.c
        sandbox/learnopengl/learnopengl.h
        sandbox/physics_for_game_developers/physics_for_game_developers.c
        sandbox/physics_for_game_developers/physics_for_game_developers.h
        sandbox/physics_for_game_developers/cannon_example.c
        sandbox/physics_for_game_developers/cannon_example.h
        sandbox/main.c
        sandbox/ui/basic_ui.c
        sandbox/ui/basic_ui.h
)

target_link_libraries(cstrl PRIVATE log.c ${OPENGL_LIBRARIES} m Xfixes)
target_link_libraries(cstrl-test PRIVATE log.c cstrl m)
target_link_libraries(cstrl-sandbox PRIVATE cstrl m)

target_include_directories(cstrl PRIVATE ${CMAKE_SOURCE_DIR}/extern ${CMAKE_SOURCE_DIR}/include)
target_include_directories(cstrl-test PRIVATE ${CMAKE_SOURCE_DIR}/extern ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
target_include_directories(cstrl-sandbox PRIVATE ${CMAKE_SOURCE_DIR}/include)
