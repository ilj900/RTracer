set(SOURCE
        tiny_obj_loader.cpp)

set(INCLUDE
        tiny_obj_loader.h)

add_library(tiny_obj_loader STATIC ${SOURCE} ${INCLUDE})

target_include_directories(RTracer PUBLIC .)