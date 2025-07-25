include_directories(
        components
        systems
        tasks
        .)

set(ECS_INCLUDE
        components/acceleration_structure_component.h
        components/device_camera_component.h
        components/device_mesh_component.h
        components/device_renderable_component.h
        components/framebuffer_component.h
        components/light_component.h
        components/material_component.h
        components/mesh_component.h
        components/transform_component.h
        components/texture_component.h

        systems/area_light_system.h
        systems/acceleration_structure_system.h
        systems/camera_system.h
        systems/directional_light_system.h
        systems/gpu_bufferable_system.h
        systems/material_system.h
        systems/mesh_system.h
        systems/point_light_system.h
        systems/renderable_system.h
        systems/spot_light_system.h
        systems/texture_system.h
        systems/transform_system.h)

set(ECS_SOURCE
        components/mesh_component.cpp

        systems/area_light_system.cpp
        systems/acceleration_structure_system.cpp
        systems/camera_system.cpp
        systems/directional_light_system.cpp
        systems/gpu_bufferable_system.cpp
        systems/material_system.cpp
        systems/mesh_system.cpp
        systems/point_light_system.cpp
        systems/renderable_system.cpp
        systems/spot_light_system.cpp
        systems/texture_system.cpp
        systems/transform_system.cpp
        )

set(INCLUDE
        named_resources.h
        render.h
        renderer_options.h
        tasks/executable_task.h
        tasks/task_update_tlas.h
        tasks/task_copy_buffer.h
        tasks/task_clear_buffer.h
        tasks/task_clear_image.h
        tasks/task_reset_active_ray_count.h
        tasks/task_raytrace.h
        tasks/task_material_sort_clear_total_materials_count.h
        tasks/task_material_sort_count_materials_per_chunk.h
        tasks/task_material_sort_compute_prefix_sums_up_sweep.h
        tasks/task_material_sort_compute_prefix_sums_zero_out.h
        tasks/task_material_sort_compute_prefix_sums_down_sweep.h
        tasks/task_material_sort_compute_offsets_per_material.h
        tasks/task_material_sort_sort_materials.h
        tasks/task_miss.h
        tasks/task_master_shader.h
        tasks/task_accumulate.h
        tasks/task_passthrough.h
        tasks/task_advance_render_count.h
        utility_functions.h
        ${ECS_INCLUDE})

set(SOURCE
        render.cpp
        renderer_options.cpp
        tasks/executable_task.cpp
        tasks/task_update_tlas.cpp
        tasks/task_copy_buffer.cpp
        tasks/task_clear_buffer.cpp
        tasks/task_clear_image.cpp
        tasks/task_reset_active_ray_count.cpp
        tasks/task_raytrace.cpp
        tasks/task_material_sort_clear_total_materials_count.cpp
        tasks/task_material_sort_count_materials_per_chunk.cpp
        tasks/task_material_sort_compute_prefix_sums_up_sweep.cpp
        tasks/task_material_sort_compute_prefix_sums_zero_out.cpp
        tasks/task_material_sort_compute_prefix_sums_down_sweep.cpp
        tasks/task_material_sort_compute_offsets_per_material.cpp
        tasks/task_material_sort_sort_materials.cpp
        tasks/task_miss.cpp
        tasks/task_master_shader.cpp
        tasks/task_accumulate.cpp
        tasks/task_passthrough.cpp
        tasks/task_advance_render_count.cpp
        utility_functions.cpp
        ${ECS_SOURCE})

add_library(Renderer ${SOURCE} ${INCLUDE})

target_include_directories(RTracer PUBLIC .)
target_include_directories(Test PUBLIC .)
target_include_directories(SceneLoader PUBLIC .)

target_link_libraries(Renderer
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        Utils
        ECS
        tinyexr
        stb_image
        tiny_obj_loader
        Vulkan_helpers)