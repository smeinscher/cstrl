cmake_minimum_required(VERSION 3.29)
project(cstrl-pong C)

# set(CMAKE_EXECUTABLE_SUFFIX ".html")

add_executable(cstrl-pong
    main.c
    pong/pong.c
    pong/pong.h
)

add_library(cstrl STATIC IMPORTED)
set_target_properties(cstrl PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIRECTORY}../libcstrl.a)

add_library(log_c STATIC IMPORTED)
set_target_properties(log_c PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIRECTORY}../liblog_c.a)

target_link_libraries(cstrl INTERFACE log_c)
target_link_libraries(cstrl-pong cstrl "-sALLOW_MEMORY_GROWTH -sMAX_WEBGL_VERSION=2 -sMIN_WEBGL_VERSION=2 --preload-file ./resources -sMODULARIZE=1")
target_include_directories(cstrl-pong PRIVATE ${CMAKE_SOURCE_DIRECTORY}../include)
