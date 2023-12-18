set(INCLUDE
        tiny_obj_loader.h)

set(SOURCE
        tiny_obj_loader.cpp)

add_library(tiny_obj_loader STATIC ${SOURCE} ${INCLUDE})

target_include_directories(Renderer PUBLIC .)