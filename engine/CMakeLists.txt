cmake_minimum_required(VERSION 4.0)

project(Engine)

include_directories(public)
include_directories(../common/external/GLFW/include)

set(CORE_SOURCES    core/Engine.cpp)

set(TIER0_SOURCES   tier0/Lexer.cpp)

set(TIER1_SOURCES   tier1/Command.cpp
                    tier1/Event.cpp
                    tier1/Cvar.cpp)

set(TIER2_SOURCES   tier2/WindowSystem.cpp)

find_package(OpenGL REQUIRED)

add_library(Engine SHARED ${CORE_SOURCES} ${TIER0_SOURCES} ${TIER1_SOURCES} ${TIER2_SOURCES})
target_link_libraries(Engine glfw ${OPENGL_LIBRARIES})