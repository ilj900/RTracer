set(INCLUDE
        application.h
        controller.h
        scene_loader.h
        swapchain.h
        task_imgui.h
        window_manager.h)

set(SOURCE
        application.cpp
        controller.cpp
        main.cpp
        scene_loader.cpp
        swapchain.cpp
        task_imgui.cpp
        window_manager.cpp)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer imgui Renderer glfw)