cmake_minimum_required(VERSION 3.20)
project(mulan)

option(MULAN_BUILD_TEST "build test app" OFF)


# mulan library
add_library(libmulan INTERFACE)
target_sources(libmulan
  INTERFACE include/mulan.h
)
target_include_directories(libmulan INTERFACE include)


# mulan compiler
add_executable(mulan)
target_link_libraries(mulan PRIVATE libmulan)
target_sources(mulan
  PRIVATE main.cc icon.h
)
target_compile_options(mulan
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
      -Wall -Werror -pedantic-errors -Wextra -Wconversion -Wsign-conversion>
    $<$<CXX_COMPILER_ID:MSVC>:
      /W4 /WX>
)


# add_mulan CMake function
function(add_mulan out in)
  set(src "${CMAKE_CURRENT_SOURCE_DIR}/${in}")
  set(dst "${CMAKE_CURRENT_BINARY_DIR}/${out}")

  add_custom_command(
    OUTPUT  "${dst}"
    DEPENDS "${src}" mulan
    COMMAND $<TARGET_FILE:mulan> compile --output "${dst}" --icon fontawesome5 "${src}"
    COMMENT "compiling ${in}..."
    VERBATIM
  )
endfunction()


if (MULAN_BUILD_TEST)
  add_subdirectory(test)
endif()
