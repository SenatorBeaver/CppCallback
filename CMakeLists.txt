cmake_minimum_required(VERSION 3.20)
project(CallbackDemo LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_VERBOSE_MAKEFILE OFF)

add_executable(${PROJECT_NAME})

add_subdirectory(src)

set(OPTIMIZATION_OPTIONS
$<$<CONFIG:RELEASE>:-Os -flto -fomit-frame-pointer>
$<$<CONFIG:DEBUG>:-O0 -g3 -ggdb>
)
set(PROJECT_COMMON_C_CPP_FLAGS
    ${CPU_OPTIONS} -Wall -Wextra -Werror=shadow -Werror=switch -Werror=return-type -Wimplicit-fallthrough=5 -ffunction-sections -fdata-sections
    ${OPTIMIZATION_OPTIONS}
)

set(PROJECT_C_FLAGS
    ${PROJECT_COMMON_C_CPP_FLAGS}
    -fno-builtin -fno-unroll-loops -ffast-math -ftree-vectorize
)
set(PROJECT_CXX_FLAGS
    ${PROJECT_COMMON_C_CPP_FLAGS} -fno-builtin -fno-unroll-loops -ffast-math -ftree-vectorize -fno-threadsafe-statics
    -fno-use-cxa-atexit -fstack-usage -Wno-register -fno-exceptions -fno-rtti
    -Wno-volatile
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD 11
    C_EXTENSIONS OFF
    CXX_STANDARD 23
    CXX_EXTENSIONS OFF
)

set(PROJECT_COMPILE_OPTIONS
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_C_FLAGS}>
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_CXX_FLAGS}>
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${PROJECT_COMPILE_OPTIONS}
)

