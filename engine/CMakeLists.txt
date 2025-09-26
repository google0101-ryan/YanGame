cmake_minimum_required(VERSION 4.0)

project(Engine)

include_directories(public)
include_directories(../common/external/GLFW/include)
include_directories(../common/external/VulkanMemoryAllocator/include)
include_directories(../common/include)
include_directories(..)

set(CORE_SOURCES    core/Engine.cpp)

set(TIER0_SOURCES   tier0/Lexer.cpp)

set(TIER1_SOURCES   tier1/Command.cpp
                    tier1/Event.cpp
                    tier1/Cvar.cpp
                    tier1/FileSystem.cpp)

set(TIER2_SOURCES   tier2/WindowSystem.cpp
                    tier2/RenderSystem.cpp
                    tier2/vulkan/RenderBackend.cpp
                    tier2/vulkan/Instance.cpp
                    tier2/vulkan/Device.cpp
                    tier2/vulkan/SwapChain.cpp
                    tier2/vulkan/Semaphore.cpp
                    tier2/vulkan/Fence.cpp
                    tier2/vulkan/RenderPass.cpp
                    tier2/vulkan/CommandBuffer.cpp
                    tier2/vulkan/Buffer.cpp
                    tier2/vulkan/StagingBuffer.cpp
                    tier2/vulkan/VMA.cpp
                    tier2/vulkan/Image.cpp)

set(COMPONENTS      components/Camera.cpp
                    components/RenderModel.cpp)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

add_library(Engine SHARED ${CORE_SOURCES} ${TIER0_SOURCES} ${TIER1_SOURCES} ${TIER2_SOURCES} ${COMPONENTS})
target_link_libraries(Engine glfw ${OPENGL_LIBRARIES} ${Vulkan_LIBRARIES})
add_compile_options(-pg)
add_link_options(-pg)