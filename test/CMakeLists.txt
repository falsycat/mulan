add_executable(mulan-test-app)
target_link_libraries(mulan-test-app
  PRIVATE libmulan
)
target_sources(mulan-test-app
  PRIVATE main.cc
)

add_mulan(ja    ja.po)
add_mulan(crazy crazy.po)

add_custom_target(mulan-test-compile ALL
  DEPENDS ja crazy
)
