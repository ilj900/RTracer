set(INCLUDE
        application.h)

set(SOURCE
        application.cpp
        main.cpp)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer Renderer)