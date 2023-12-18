set(INCLUDE
        application.h
        controller.h)

set(SOURCE
        application.cpp
        controller.cpp
        main.cpp)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer Renderer)