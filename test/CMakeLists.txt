add_executable(mulan-test-app)
target_link_libraries(mulan-test-app
  PRIVATE libmulan
)
target_sources(mulan-test-app
  PRIVATE main.cc
)
target_compile_options(mulan-test-app
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
      -Wall -Werror -pedantic-errors -Wextra -Wconversion -Wsign-conversion>
    $<$<CXX_COMPILER_ID:MSVC>:
      /W4 /WX>
)


add_mulan(ja    ja.po)
add_mulan(crazy crazy.po)

add_custom_target(mulan-test-compile ALL
  DEPENDS ja crazy
)
