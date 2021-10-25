cmake_minimum_required(VERSION 3.13)

project(RTracer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(glfw)

include_directories(
        glfw/include
        vulkan_helpers
        ${Vulkan}/Include
        include
        math
        src
        ECS
        components
        systems)

set(ECS_SOURCE
        ECS/coordinator.h
        ECS/coordinator.cpp
        ECS/entity_manager.h
        ECS/entity_manager.cpp
        ECS/component_array.h
        ECS/component_manager.h
        ECS/system_manager.h
        ECS/entities/entity.h
        ECS/components/transform_component.h
        ECS/components/camera_component.h
        ECS/components/device_camera_component.h
        ECS/components/renderable_component.h
        ECS/components/mesh_component.h
        ECS/components/device_mesh_component.h
        ECS/components/mesh_component.cpp
        ECS/systems/system.h
        ECS/systems/camera_system.h
        ECS/systems/camera_system.cpp
        ECS/systems/transform_system.h
        ECS/systems/transform_system.cpp
        ECS/systems/renderable_system.h
        ECS/systems/renderable_system.cpp
        ECS/systems/mesh_system.h
        ECS/systems/mesh_system.cpp)

set(SOURCE
        src/main.cpp
        src/controller.cpp
        vulkan_helpers/context.cpp
        vulkan_helpers/resource_allocation.cpp
        vulkan_helpers/function_loader.cpp
        vulkan_helpers/swapchain.cpp
        math/maths.cpp)

set(INCLUDE
        ${ECS_SOURCE}
        src/controller.h
        vulkan_helpers/context.h
        vulkan_helpers/resource_allocation.h
        vulkan_helpers/function_loader.h
        vulkan_helpers/swapchain.h
        vulkan_helpers/buffer.h
        include/stb_image.h
        include/tiny_obj_loader.h
        math/maths.h)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)