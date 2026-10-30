cmake_minimum_required(VERSION 3.16)
project(Nirea VERSION 0.7.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# The runtime is embedded in the compiler and written next to generated C.
set(RUNTIME_H ${CMAKE_CURRENT_SOURCE_DIR}/runtime/nirea_rt.h)
set(RUNTIME_C ${CMAKE_CURRENT_SOURCE_DIR}/runtime/nirea_rt.c)
set(EMBEDDED_RUNTIME ${CMAKE_CURRENT_BINARY_DIR}/embedded_runtime.c)
add_custom_command(
    OUTPUT ${EMBEDDED_RUNTIME}
    COMMAND ${CMAKE_COMMAND} -DOUTPUT=${EMBEDDED_RUNTIME}
            "-DINPUTS=nirea_rt_h=${RUNTIME_H}\;nirea_rt_c=${RUNTIME_C}"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    DEPENDS ${RUNTIME_H} ${RUNTIME_C} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    COMMENT "Embedding the Nirea runtime")

add_executable(nirea
    src/main.c
    src/common.c
    src/lexer.c
    src/parser.c
    src/types.c
    src/sema.c
    src/ownership.c
    src/codegen.c
    src/driver.c
    ${EMBEDDED_RUNTIME})
target_include_directories(nirea PRIVATE src)
target_compile_definitions(nirea PRIVATE _POSIX_C_SOURCE=200809L _DEFAULT_SOURCE)

install(TARGETS nirea RUNTIME DESTINATION bin)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
