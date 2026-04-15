add_executable(duckstation-sdl
  sdl_host.cpp
)

if(BUILD_SDL_FRONTEND AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CROSSCOMPILING)
  target_sources(duckstation-sdl PRIVATE glibc_compat.c)
  set_source_files_properties(glibc_compat.c PROPERTIES COMPILE_FLAGS "-Wno-deprecated")
endif()

target_include_directories(duckstation-sdl PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(duckstation-sdl PRIVATE core common scmversion)

add_core_resources(duckstation-sdl)
