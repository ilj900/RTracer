set(INCLUDE
        application.h
        swapchain.h
        window_manager.h)

set(SOURCE
        application.cpp
        main.cpp
        swapchain.cpp
        window_manager.cpp)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer Renderer)