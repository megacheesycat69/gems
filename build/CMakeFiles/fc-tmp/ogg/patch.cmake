cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

message(VERBOSE "Executing patch step for ogg")

block(SCOPE_FOR VARIABLES)

execute_process(
  WORKING_DIRECTORY "C:/gems_game/build/_deps/ogg-src"
  COMMAND_ERROR_IS_FATAL LAST
  COMMAND  [====[C:/Program Files/CMake/bin/cmake.exe]====] [====[-DOGG_DIR=C:/gems_game/build/_deps/ogg-src]====] [====[-P]====] [====[C:/gems_game/build/_deps/sfml-src/tools/ogg/PatchOgg.cmake]====]
)

endblock()
