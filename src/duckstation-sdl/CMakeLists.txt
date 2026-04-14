add_executable(duckstation-sdl
  sdl_host.cpp
)

target_include_directories(duckstation-sdl PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(duckstation-sdl PRIVATE core common scmversion)

add_core_resources(duckstation-sdl)
