cmake_minimum_required(VERSION 3.29)
project(cstrl C)
project(cstrl-test C)

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
        src/util/dynamic_array.h
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
        src/renderer/opengl/opengl_shader.h
        src/renderer/opengl/opengl_texture.c
        src/renderer/opengl/opengl_texture.h
        src/util/file_helpers.c
        src/util/file_helpers.h
        extern/stb/stb_image.c
        extern/stb/stb_image.h
        src/renderer/opengl/opengl_shader_programs.h
        src/renderer/camera.c
        src/renderer/camera.h
        src/platform/platform_common.c
)

add_executable(cstrl-test
        test/main.c
        test/test_manager.c
        test/test_manager.h
        test/cstrl_math/test_vec2.c
        test/cstrl_math/test_vec2.h
        test/test_types.h
        test/cstrl_math/test_vec3.c
        test/cstrl_math/test_vec3.h
        test/cstrl_math/test_vec4.c
        test/cstrl_math/test_vec4.h
        test/util/test_dynamic_array.c
        test/util/test_dynamic_array.h
        test/cstrl_math/test_quat.c
        test/cstrl_math/test_quat.h
)

target_link_libraries(cstrl PRIVATE log.c ${OPENGL_LIBRARIES} m)
target_link_libraries(cstrl-test PRIVATE log.c cstrl m)

target_include_directories(cstrl PRIVATE ${CMAKE_SOURCE_DIR}/extern ${CMAKE_SOURCE_DIR}/include)
target_include_directories(cstrl-test PRIVATE ${CMAKE_SOURCE_DIR}/extern ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
