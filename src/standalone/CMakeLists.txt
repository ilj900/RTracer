set(INCLUDE
        application.h
        controller.h)

set(SOURCE
        application.cpp
        controller.cpp
        main.cpp)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer
        Math
        Utils
        ECS
        tinyexr
        stb_image
        tiny_obj_loader
        imgui
        Vulkan_helpers
        Renderer)