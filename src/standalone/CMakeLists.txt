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

set(SOURCE
        application.h
        application.cpp
        main.cpp
        controller.cpp
        controller.h)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer
        glfw
        Math
        Utils
        ECS
        tinyexr
        stb_image
        tiny_obj_loader
        imgui
        Vulkan_helpers
        Renderer)