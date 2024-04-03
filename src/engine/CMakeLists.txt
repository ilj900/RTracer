include_directories(
        components
        systems)

set(ECS_INCLUDE
        components/acceleration_structure_component.h
        components/camera_component.h
        components/device_camera_component.h
        components/device_mesh_component.h
        components/device_renderable_component.h
        components/light_component.h
        components/material_component.h
        components/mesh_component.h
        components/transform_component.h

        systems/acceleration_structure_system.h
        systems/camera_system.h
        systems/gpu_bufferable_system.h
        systems/light_system.h
        systems/material_system.h
        systems/mesh_system.h
        systems/renderable_system.h
        systems/transform_system.h)

set(ECS_SOURCE
        components/mesh_component.cpp

        systems/acceleration_structure_system.cpp
        systems/camera_system.cpp
        systems/gpu_bufferable_system.cpp
        systems/light_system.cpp
        systems/material_system.cpp
        systems/mesh_system.cpp
        systems/renderable_system.cpp
        systems/transform_system.cpp
        )

set(INCLUDE
        controller.h
        executable_task.h
        render.h
        task_accumulate.h
        task_clear_image.h
        task_imgui.h
        task_material_sort_clear_materials_count_per_chunk.h
        task_material_sort_clear_total_materials_count.h
        task_material_sort_compute_offsets_per_material.h
        task_material_sort_count_materials_per_chunk.h
        task_material_sort_sort_materials.h
        task_material_sort_compute_prefix_sums_down_sweep.h
        task_material_sort_compute_prefix_sums_up_sweep.h
        task_material_sort_compute_prefix_sums_zero_out.h
        task_passthrough.h
        task_raytrace.h
        task_shade.h
        window_manager.h
        ${ECS_INCLUDE})

set(SOURCE
        controller.cpp
        executable_task.cpp
        task_generate_initial_rays.h
        task_passthrough.cpp
        task_imgui.cpp
        task_material_sort_clear_materials_count_per_chunk.cpp
        task_material_sort_clear_total_materials_count.cpp
        task_material_sort_compute_offsets_per_material.cpp
        task_material_sort_count_materials_per_chunk.cpp
        task_material_sort_sort_materials.cpp
        task_material_sort_compute_prefix_sums_down_sweep.cpp
        task_material_sort_compute_prefix_sums_up_sweep.cpp
        task_material_sort_compute_prefix_sums_zero_out.cpp
        task_raytrace.cpp
        task_accumulate.cpp
        task_clear_image.cpp
        task_shade.cpp
        task_generate_initial_rays.cpp
        render.cpp
        window_manager.cpp
        ${ECS_SOURCE})

add_library(Renderer ${SOURCE} ${INCLUDE})

target_include_directories(RTracer PUBLIC .)

target_link_libraries(Renderer
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        Utils
        ECS
        tinyexr
        stb_image
        tiny_obj_loader
        imgui
        Vulkan_helpers)