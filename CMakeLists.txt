cmake_minimum_required(VERSION 3.24)

project(RTracer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

add_subdirectory(glfw)

include_directories(
        glfw/include
        vulkan_helpers
        ${Vulkan_INCLUDE_DIRS}
        include
        math
        src
        utils
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
        ECS/component_manager.cpp
        ECS/system_manager.h
        ECS/entities/entity.h

        ECS/components/transform_component.h
        ECS/components/camera_component.h
        ECS/components/device_camera_component.h
        ECS/components/device_renderable_component.h
        ECS/components/material_component.h
        ECS/components/mesh_component.h
        ECS/components/device_mesh_component.h
        ECS/components/mesh_component.cpp
        ECS/components/light_component.h

        ECS/systems/system.h
        ECS/systems/system.cpp
        ECS/systems/camera_system.h
        ECS/systems/camera_system.cpp
        ECS/systems/transform_system.h
        ECS/systems/transform_system.cpp
        ECS/systems/renderable_system.h
        ECS/systems/renderable_system.cpp
        ECS/systems/material_system.h
        ECS/systems/material_system.cpp
        ECS/systems/mesh_system.h
        ECS/systems/mesh_system.cpp
        ECS/systems/light_system.h
        ECS/systems/light_system.cpp
        ECS/systems/gpu_bufferable_system.h
        ECS/systems/gpu_bufferable_system.cpp)

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
        src/task_generate_initial_rays.h
        src/task_generate_initial_rays.cpp
        src/tinyexr.cpp
        vulkan_helpers/vk_acceleration_structure.h
        vulkan_helpers/vk_context.h
        vulkan_helpers/vk_utils.cpp
        vulkan_helpers/vk_context.cpp
        vulkan_helpers/image.cpp
        vulkan_helpers/resource_allocation.cpp
        vulkan_helpers/swapchain.cpp
        vulkan_helpers/renderpass.cpp
        vulkan_helpers/descriptors.cpp
        vulkan_helpers/command_buffer_manager.cpp
        vulkan_helpers/vk_shader_compiler.cpp
        math/maths.cpp
        utils/logging.h
        utils/logging.cpp
        include/miniz.c
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
        vulkan_helpers/buffer.cpp
        vulkan_helpers/descriptors.h
        vulkan_helpers/command_buffer_manager.h
        vulkan_helpers/vk_pipeline.h
        vulkan_helpers/vk_pipeline.cpp
        vulkan_helpers/vk_debug.h
        vulkan_helpers/vk_debug.cpp
        vulkan_helpers/vk_functions.h
        vulkan_helpers/vk_functions.cpp
        vulkan_helpers/vk_shader_compiler.h
        include/stb_image.h
        include/stb_image_write.h
        include/tiny_obj_loader.h
        include/tinyexr.h
        include/miniz.c
        math/maths.h)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        glfw)