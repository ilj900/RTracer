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

set(INCLUDE
        application.h
        controller.h)

set(SOURCE
        application.cpp
        controller.cpp
        main.cpp)

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