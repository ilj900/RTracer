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
        src/ECS
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
        src/task_generate_initial_rays.cpp)

set(INCLUDE
        ${ECS_SOURCE}
        src/controller.h)

add_executable(RTracer ${SOURCE} ${INCLUDE})

add_subdirectory(src/vulkan_helpers)
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
        imgui
        Vulkan_helpers)