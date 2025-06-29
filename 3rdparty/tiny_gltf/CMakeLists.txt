set(INCLUDE
        tiny_gltf.h
        json.hpp
        stb_image.h
        stb_image_write.h)

set(SOURCE
        tiny_gltf.cpp)

add_library(tiny_gltf STATIC ${SOURCE} ${INCLUDE})

target_include_directories(Renderer PUBLIC .)