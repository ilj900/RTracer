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
        imgui
        imgui/backends
        components
        systems)

set(IMGUI_SOURCE
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_vulkan.cpp
        imgui/imgui.cpp imgui/imgui_draw.cpp
        imgui/imgui_demo.cpp imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp)

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
        ECS/components/device_renderable_component.h
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
        vulkan_helpers/vk_context.h
        vulkan_helpers/vk_utils.cpp
        vulkan_helpers/vk_context.cpp
        vulkan_helpers/image.cpp
        vulkan_helpers/resource_allocation.cpp
        vulkan_helpers/swapchain.cpp
        vulkan_helpers/renderpass.cpp
        vulkan_helpers/descriptors.cpp
        vulkan_helpers/command_buffer_manager.cpp
        math/maths.cpp
        ${IMGUI_SOURCE})

set(INCLUDE
        ${ECS_SOURCE}
        src/controller.h
        vulkan_helpers/vk_utils.h
        vulkan_helpers/vk_context.h
        vulkan_helpers/image.h
        vulkan_helpers/resource_allocation.h
        vulkan_helpers/swapchain.h
        vulkan_helpers/renderpass.h
        vulkan_helpers/buffer.h
        vulkan_helpers/descriptors.h
        vulkan_helpers/command_buffer_manager.h
        vulkan_helpers/vk_pipeline.h
        vulkan_helpers/vk_pipeline.cpp
        vulkan_helpers/vk_debug.h
        vulkan_helpers/vk_debug.cpp
        vulkan_helpers/vk_functions.h
        vulkan_helpers/vk_functions.cpp
        include/stb_image.h
        include/stb_image_write.h
        include/tiny_obj_loader.h
        math/maths.h)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)