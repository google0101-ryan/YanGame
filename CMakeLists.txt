cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_STANDARD 23)

project(YanGame)

add_subdirectory(common/external/GLFW)

add_subdirectory(engine)
add_subdirectory(app/editor)