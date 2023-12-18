set(INCLUDE
        maths.h)

set(SOURCE
        maths.cpp)

add_library(Math STATIC ${SOURCE} ${INCLUDE})

target_include_directories(Renderer PUBLIC .)
target_include_directories(Vulkan_helpers PUBLIC .)