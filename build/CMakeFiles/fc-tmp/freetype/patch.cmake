cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

message(VERBOSE "Executing patch step for freetype")

block(SCOPE_FOR VARIABLES)

execute_process(
  WORKING_DIRECTORY "C:/gems_game/build/_deps/freetype-src"
  COMMAND_ERROR_IS_FATAL LAST
  COMMAND  [====[C:/Program Files/CMake/bin/cmake.exe]====] [====[-DFREETYPE_DIR=C:/gems_game/build/_deps/freetype-src]====] [====[-P]====] [====[C:/gems_game/build/_deps/sfml-src/tools/freetype/PatchFreetype.cmake]====]
)

endblock()
