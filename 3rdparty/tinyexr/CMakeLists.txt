set(INCLUDE
        tinyexr.h
        miniz.h)

set(SOURCE
        tinyexr.cpp
        miniz.c)

add_library(tinyexr STATIC ${SOURCE} ${INCLUDE})

target_include_directories(Renderer PUBLIC .)
target_include_directories(Vulkan_helpers PUBLIC .)
target_include_directories(tinyexr PRIVATE .)