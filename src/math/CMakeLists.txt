target_include_directories(RTracer PUBLIC .)
target_include_directories(Vulkan_helpers PUBLIC .)

set(SOURCE
        maths.cpp)

set(INCLUDE
        maths.h)

add_library(Math STATIC ${SOURCE} ${INCLUDE})