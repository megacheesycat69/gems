cmake_minimum_required(VERSION 3.16)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_NAME GEMS)

project(${PROJECT_NAME} LANGUAGES C CXX)

# Adding SFML 3.1.0
FetchContent_Declare(
  sfml
  GIT_REPOSITORY "https://github.com/SFML/SFML.git"
  GIT_TAG "3.1.0"
)
FetchContent_MakeAvailable(sfml)

add_executable(${PROJECT_NAME} Source/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-system sfml-window)

target_include_directories(${PROJECT_NAME} PUBLIC Source/Public)
target_sources(${PROJECT_NAME} PRIVATE
  Source/Private/Game.cpp
  Source/Private/Board.cpp
)
