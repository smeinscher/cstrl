cmake_minimum_required(VERSION 3.29)
project(cstrl C)
project(cstrl-test C)
project(cstrl-sandbox C)

#set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)

find_package(OpenGL)

find_package(Vulkan)

find_package(X11 QUIET)

add_library(log_c STATIC
        extern/log.c/log.c
        extern/log.c/log.h)
target_compile_options(log_c PRIVATE -DLOG_USE_COLOR)

if (NOT DEFINED RENDER_API)
    set(RENDER_API "OPENGL")
endif ()

if (${RENDER_API} STREQUAL "OPENGL")
    add_compile_definitions(CSTRL_RENDER_API_OPENGL)
elseif (${RENDER_API} STREQUAL "VULKAN")
    add_compile_definitions(CSTRL_RENDER_API_VULKAN)
endif ()

if (ANDROID)
    set(GLAD_C extern/glad/gles3/glad.c)
    set(GLAD_H extern/glad/gles3/glad.h extern/glad/gles3/KHR/khrplatform.h)
else ()
    set(GLAD_C extern/glad/glad.c)
    set(GLAD_H extern/glad/glad.h)
endif ()

add_library(cstrl SHARED
        ${GLAD_C}
        ${GLAD_H}
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
        extern/stb/stb_truetype.c
        extern/stb/stb_truetype.h
        extern/stb/stb_image_write.c
        extern/stb/stb_image_write.h
        src/platform/platform_android.c
        src/renderer/opengl/opengl_platform_android.c
        src/renderer/vulkan/vulkan_renderer.c
        src/renderer/vulkan/vulkan_shader.c
        src/renderer/vulkan/vulkan_texture.c
        include/cstrl/cstrl_camera.h
        src/renderer/vulkan/vulkan_platform.h
        src/renderer/vulkan/vulkan_platform_win32.c
        src/renderer/vulkan/vulkan_platform_linux.c
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
        sandbox/vulkan-tutorial/vulkan_triangle.c
        sandbox/vulkan-tutorial/vulkan_triangle.h
        sandbox/the_ray_tracer_challenge/challenge.c
        sandbox/the_ray_tracer_challenge/challenge.h
        sandbox/pong/pong.c
        sandbox/pong/pong.h
)

target_compile_options(cstrl PRIVATE -DCSTRL_EXPORT)
target_link_libraries(cstrl PRIVATE log_c ${OPENGL_LIBRARIES})
if (X11_FOUND)
    target_link_libraries(cstrl PRIVATE Xfixes)
endif ()
if (UNIX)
    target_link_libraries(cstrl PRIVATE m)
    target_link_libraries(cstrl-test PRIVATE m)
    target_link_libraries(cstrl-sandbox PRIVATE m)
endif ()
if (ANDROID)
    target_link_libraries(cstrl PRIVATE EGL)
else ()
    target_link_libraries(cstrl PRIVATE Vulkan::Vulkan)
endif ()
target_link_libraries(cstrl-test PRIVATE log_c cstrl)
target_link_libraries(cstrl-sandbox PRIVATE cstrl)

target_include_directories(cstrl PRIVATE ${CMAKE_SOURCE_DIR}/extern ${CMAKE_SOURCE_DIR}/include)
target_include_directories(cstrl-test PRIVATE ${CMAKE_SOURCE_DIR}/extern ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
target_include_directories(cstrl-sandbox PRIVATE ${CMAKE_SOURCE_DIR}/include)

