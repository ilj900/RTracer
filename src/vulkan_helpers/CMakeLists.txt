set(INCLUDE
        buffer.h
        command_buffer_manager.h
        descriptors.h
        image.h
        resource_allocation.h
        swapchain.h
        texture_manager.h
        timing_manager.h
        vk_acceleration_structure.h
        vk_context.h
        vk_debug.h
        vk_functions.h
        vk_pipeline.h
        vk_shader_compiler.h
        vk_utils.h)

set(SOURCE
        buffer.cpp
        command_buffer_manager.cpp
        descriptors.cpp
        image.cpp
        resource_allocation.cpp
        swapchain.cpp
        texture_manager.cpp
        timing_manager.cpp
        vk_context.cpp
        vk_debug.cpp
        vk_functions.cpp
        vk_pipeline.cpp
        vk_shader_compiler.cpp
        vk_utils.cpp)

add_library(Vulkan_helpers STATIC ${SOURCE} ${INCLUDE})

target_link_libraries(Vulkan_helpers Math Vulkan::Vulkan glfw)

target_include_directories(Renderer PUBLIC .)