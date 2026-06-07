cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

message(VERBOSE "Executing patch step for libssh2")

block(SCOPE_FOR VARIABLES)

execute_process(
  WORKING_DIRECTORY "C:/gems_game/build/_deps/libssh2-src"
  COMMAND_ERROR_IS_FATAL LAST
  COMMAND  [====[C:/Program Files/CMake/bin/cmake.exe]====] [====[-DLIBSSH2_DIR=C:/gems_game/build/_deps/libssh2-src]====] [====[-DMODULES_DIR=C:/gems_game/build/_deps/sfml-src/src/SFML/Network/../../../cmake/Modules]====] [====[-P]====] [====[C:/gems_game/build/_deps/sfml-src/tools/libssh2/PatchLibssh2.cmake]====]
)

endblock()
