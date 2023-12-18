set(SOURCE
        vk_acceleration_structure.h
        vk_context.h
        vk_utils.cpp
        vk_context.cpp
        image.cpp
        resource_allocation.cpp
        swapchain.cpp
        renderpass.cpp
        descriptors.cpp
        command_buffer_manager.cpp
        vk_shader_compiler.cpp
        texture_manager.h
        texture_manager.cpp)

set(INCLUDE
        vk_utils.h
        vk_context.h
        image.h
        resource_allocation.h
        swapchain.h
        renderpass.h
        buffer.h
        buffer.cpp
        descriptors.h
        command_buffer_manager.h
        vk_pipeline.h
        vk_pipeline.cpp
        vk_debug.h
        vk_debug.cpp
        vk_functions.h
        vk_functions.cpp
        vk_shader_compiler.h)

add_library(Vulkan_helpers STATIC ${SOURCE} ${INCLUDE})

target_link_libraries(Vulkan_helpers Math Vulkan::Vulkan glfw)

target_include_directories(RTracer PUBLIC .)