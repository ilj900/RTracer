set(INCLUDE
        application.h
        swapchain.h)

set(SOURCE
        application.cpp
        main.cpp
        swapchain.cpp)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer Renderer)