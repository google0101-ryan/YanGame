cmake_minimum_required(VERSION 4.0)

project(Editor)

set(EDITOR_SOURCES  EditorApp.cpp)

include_directories(../../engine/public)
include_directories(../../common/include)
add_executable(Editor ${EDITOR_SOURCES})

target_link_libraries(Editor Engine)