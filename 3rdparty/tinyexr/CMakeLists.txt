set(SOURCE
        tinyexr.cpp
        miniz.c)

set(INCLUDE
        tinyexr.h
        miniz.h)

add_library(tinyexr STATIC ${SOURCE} ${INCLUDE})

target_include_directories(RTracer PUBLIC .)
target_include_directories(Vulkan_helpers PUBLIC .)
target_include_directories(tinyexr PRIVATE .)