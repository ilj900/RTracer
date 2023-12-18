cmake_minimum_required(VERSION 3.24)

project(RTracer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

add_subdirectory(3rdparty/glfw)

include_directories(
        3rdparty/glfw/include
        src/vulkan_helpers
        ${Vulkan_INCLUDE_DIRS}
        include
        src
        utils
        ECS
        3rdparty/imgui
        3rdparty/imgui/backends
        components
        systems
        shaders)

set(ECS_SOURCE

        src/components/transform_component.h
        src/components/camera_component.h
        src/components/device_camera_component.h
        src/components/device_renderable_component.h
        src/components/material_component.h
        src/components/mesh_component.h
        src/components/device_mesh_component.h
        src/components/mesh_component.cpp
        src/components/light_component.h

        src/systems/camera_system.h
        src/systems/camera_system.cpp
        src/systems/transform_system.h
        src/systems/transform_system.cpp
        src/systems/renderable_system.h
        src/systems/renderable_system.cpp
        src/systems/material_system.h
        src/systems/material_system.cpp
        src/systems/mesh_system.h
        src/systems/mesh_system.cpp
        src/systems/light_system.h
        src/systems/light_system.cpp
        src/systems/gpu_bufferable_system.h
        src/systems/gpu_bufferable_system.cpp)

set(SOURCE
        src/application.h
        src/application.cpp
        src/render.h
        src/render.cpp
        src/main.cpp
        src/controller.cpp
        src/executable_task.h
        src/executable_task.cpp
        src/task_passthrough.h
        src/task_passthrough.cpp
        src/task_imgui.h
        src/task_imgui.cpp
        src/task_raytrace.h
        src/task_raytrace.cpp
        src/task_accumulate.h
        src/task_accumulate.cpp
        src/task_clear_image.h
        src/task_clear_image.cpp
        src/task_shade.h
        src/task_shade.cpp
        src/task_generate_initial_rays.h
        src/task_generate_initial_rays.cpp
        src/vulkan_helpers/vk_acceleration_structure.h
        src/vulkan_helpers/vk_context.h
        src/vulkan_helpers/vk_utils.cpp
        src/vulkan_helpers/vk_context.cpp
        src/vulkan_helpers/image.cpp
        src/vulkan_helpers/resource_allocation.cpp
        src/vulkan_helpers/swapchain.cpp
        src/vulkan_helpers/renderpass.cpp
        src/vulkan_helpers/descriptors.cpp
        src/vulkan_helpers/command_buffer_manager.cpp
        src/vulkan_helpers/vk_shader_compiler.cpp
        src/vulkan_helpers/texture_manager.h
        src/vulkan_helpers/texture_manager.cpp)

set(INCLUDE
        ${ECS_SOURCE}
        src/controller.h
        src/vulkan_helpers/vk_utils.h
        src/vulkan_helpers/vk_context.h
        src/vulkan_helpers/image.h
        src/vulkan_helpers/resource_allocation.h
        src/vulkan_helpers/swapchain.h
        src/vulkan_helpers/renderpass.h
        src/vulkan_helpers/buffer.h
        src/vulkan_helpers/buffer.cpp
        src/vulkan_helpers/descriptors.h
        src/vulkan_helpers/command_buffer_manager.h
        src/vulkan_helpers/vk_pipeline.h
        src/vulkan_helpers/vk_pipeline.cpp
        src/vulkan_helpers/vk_debug.h
        src/vulkan_helpers/vk_debug.cpp
        src/vulkan_helpers/vk_functions.h
        src/vulkan_helpers/vk_functions.cpp
        src/vulkan_helpers/vk_shader_compiler.h)

add_executable(RTracer ${SOURCE} ${INCLUDE})

add_subdirectory(src/ECS)
add_subdirectory(src/math)
add_subdirectory(src/shaders)
add_subdirectory(src/utils)
add_subdirectory(3rdparty)

target_link_libraries(RTracer
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        glfw
        Math
        Utils
        ECS
        tinyexr
        stb_image
        tiny_obj_loader
        imgui)