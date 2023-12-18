set(SOURCE
        stb_image.cpp
        stb_image_write.cpp)

set(INCLUDE
        stb_image.h
        stb_image_write.h)

add_library(stb_image STATIC ${SOURCE} ${INCLUDE})

target_include_directories(RTracer PUBLIC .)
target_include_directories(Vulkan_helpers PUBLIC .)
target_include_directories(stb_image PRIVATE .)