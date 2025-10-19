cmake_minimum_required(VERSION 3.29)
project(cstrl C)

#set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

find_package(OpenGL)

find_package(Vulkan)

if (WIN32)
    set(OPENAL_SDK "C:/Program Files (x86)/OpenAL 1.1 SDK")
    include_directories("${OPENAL_SDK}/include")
    link_directories("${OPENAL_SDK}/libs/Win64")
    set(OPENAL_LIBRARY "OpenAL32.lib")
else ()
    find_package(OpenAL)
    set(OPENAL_LIBRARY OpenAL::OpenAL)
endif ()

find_package(X11 QUIET)

add_library(log_c STATIC
        extern/log.c/log.c
        extern/log.c/log.h)
if (UNIX)
    target_compile_options(log_c PRIVATE -DLOG_USE_COLOR -fPIC)
else ()
    target_compile_options(log_c PRIVATE -DLOG_USE_COLOR)
endif()

if (NOT DEFINED RENDER_API)
    set(RENDER_API "OPENGL")
endif ()

if (${RENDER_API} STREQUAL "OPENGL")
    add_compile_definitions(CSTRL_RENDER_API_OPENGL)
elseif (${RENDER_API} STREQUAL "VULKAN")
    add_compile_definitions(CSTRL_RENDER_API_VULKAN)
endif ()

if (ANDROID OR EMSCRIPTEN)
    set(GLAD_C extern/glad/gles3/glad.c)
    set(GLAD_H extern/glad/gles3/glad.h extern/glad/gles3/KHR/khrplatform.h)
else ()
    set(GLAD_C extern/glad/glad.c)
    set(GLAD_H extern/glad/glad.h)
endif ()

add_library(cstrl SHARED
        ${GLAD_C}
        ${GLAD_H}
        extern/stb/stb_image.c
        extern/stb/stb_image.h
        # TODO: find a way for this not to clash with nuklear
        # extern/stb/stb_truetype.c
        # extern/stb/stb_truetype.h
        extern/stb/stb_image_write.c
        extern/stb/stb_image_write.h
        extern/cgltf/cgltf.c
        extern/cgltf/cgltf.h
        extern/fast_obj/fast_obj.c
        extern/fast_obj/fast_obj.h
        # extern/nuklear/nuklear.h
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
        src/renderer/glsl_shader_programs.h
        src/renderer/camera.c
        src/platform/platform_common.c
        src/util/general_helpers.c
        # TODO: find a way for this not to clash with nuklear
        # include/cstrl/cstrl_ui.h
        # src/ui/ui.c
        # src/ui/ui_opengl_renderer.c
        # src/ui/ui_internal.h
        src/platform/platform_android.c
        src/renderer/opengl/opengl_platform_android.c
        src/renderer/vulkan/vulkan_renderer.c
        src/renderer/vulkan/vulkan_shader.c
        src/renderer/vulkan/vulkan_texture.c
        include/cstrl/cstrl_camera.h
        src/renderer/vulkan/vulkan_platform.h
        src/renderer/vulkan/vulkan_platform_win32.c
        src/renderer/vulkan/vulkan_platform_linux.c
        include/cstrl/cstrl_physics.h
        src/physics/collision.c
        src/debug/assert.c
        src/platform/platform_em_web.c
        src/renderer/opengl/opengl_platform_em_web.c
        src/audio/openal/openal_audio.c
        include/cstrl/cstrl_audio.h
        src/util/random.c
        src/model/gltf_loader.c
        include/cstrl/cstrl_model.h
        src/model/obj_loader.c
        src/ui/nuklear/nuklear_cstrl.c
        include/cstrl/cstrl_nuklear_ui.h
)

# target_compile_options(cstrl PRIVATE -Wall -Werror -Wno-missing-braces -DCSTRL_EXPORT)
target_compile_options(cstrl PRIVATE -DCSTRL_EXPORT)

target_link_libraries(cstrl PRIVATE log_c ${OPENGL_LIBRARIES} ${OPENAL_LIBRARY})
if (X11_FOUND)
    target_link_libraries(cstrl PRIVATE Xfixes)
endif ()
if (UNIX)
    target_link_libraries(cstrl PRIVATE m)
endif ()
if (ANDROID)
    target_link_libraries(cstrl PRIVATE EGL)
endif ()

target_include_directories(cstrl PUBLIC extern include)
