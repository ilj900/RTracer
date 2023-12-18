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
        systems)

set(ECS_INCLUDE
        components/transform_component.h
        components/camera_component.h
        components/device_camera_component.h
        components/device_renderable_component.h
        components/material_component.h
        components/mesh_component.h
        components/light_component.h
        components/device_mesh_component.h

        systems/camera_system.h
        systems/transform_system.h
        systems/renderable_system.h
        systems/material_system.h
        systems/mesh_system.h
        systems/light_system.h
        systems/gpu_bufferable_system.h)

set(ECS_SOURCE
        components/mesh_component.cpp

        systems/camera_system.cpp
        systems/transform_system.cpp
        systems/renderable_system.cpp
        systems/material_system.cpp
        systems/mesh_system.cpp
        systems/light_system.cpp
        systems/gpu_bufferable_system.cpp)

set(INCLUDE
        executable_task.h
        task_passthrough.h
        task_imgui.h
        task_raytrace.h
        task_accumulate.h
        task_clear_image.h
        task_shade.h
        render.h
        ${ECS_INCLUDE})

set(SOURCE
        executable_task.cpp
        task_generate_initial_rays.h
        task_passthrough.cpp
        task_imgui.cpp
        task_raytrace.cpp
        task_accumulate.cpp
        task_clear_image.cpp
        task_shade.cpp
        task_generate_initial_rays.cpp
        render.cpp
        ${ECS_SOURCE})

add_library(Renderer ${SOURCE} ${INCLUDE})

target_include_directories(RTracer PUBLIC .)

target_link_libraries(Renderer
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        glfw
        Utils
        ECS
        tinyexr
        stb_image
        tiny_obj_loader
        imgui
        Vulkan_helpers)