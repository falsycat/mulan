cmake_minimum_required(VERSION 3.20)
project(mulan)

option(MULAN_BUILD_TEST "build test app" OFF)


# mulan compiler
add_executable(mulan)
target_sources(mulan
  PRIVATE main.c mulan.h
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
    DEPENDS "${src}"
    COMMAND $<TARGET_FILE:mulan> compile "${dst}" "${src}"
    COMMENT "compiling ${in}..."
    VERBATIM
  )
endfunction()


if (MULAN_BUILD_TEST)
  add_subdirectory(test)
endif()
