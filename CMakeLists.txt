cmake_minimum_required(VERSION 3.1)
project(chippie-8 C CXX)

set(CMAKE_CXX_STANDARD 17)

# Setting parameters for raylib
#set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
#set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games


add_executable(${PROJECT_NAME} src/main.cpp)
# static libraries
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
# local header/library
target_include_directories(chippie-8 PRIVATE include)

# Set the asset path macro to the absolute path on the dev machine
target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

# Set the asset path macro in release mode
#target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="relative-path-to-assets-in-the-game-package")
