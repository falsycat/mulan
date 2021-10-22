cmake_minimum_required(VERSION 3.20)
project(mulan)

option(MULAN_BUILD_TEST "build test app" OFF)


# mulan compiler
add_executable(mulan)
target_sources(mulan
  PRIVATE main.c mulan.h
)
target_compile_options(mulan
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
      -Wall -Wextra -Werror -pedantic -Wno-missing-field-initializers>
    $<$<CXX_COMPILER_ID:MSVC>:
      /W2 /std:c11 /Zc:preprocessor->
)


# mulan library
add_library(libmulan INTERFACE)
target_sources(libmulan
  INTERFACE mulan.h
)
target_include_directories(libmulan INTERFACE .)


# add_mulan CMake function
function(add_mulan out in)
  set(src "${CMAKE_CURRENT_SOURCE_DIR}/${in}")
  set(dst "${CMAKE_CURRENT_BINARY_DIR}/${out}")

  add_custom_command(
    OUTPUT  "${dst}"
    DEPENDS "${src}" mulan
    COMMAND $<TARGET_FILE:mulan> compile "${dst}" "${src}"
    COMMENT "compiling ${in}..."
    VERBATIM
  )
endfunction()


if (MULAN_BUILD_TEST)
  add_subdirectory(test)
endif()
