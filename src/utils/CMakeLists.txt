target_include_directories(RTracer PUBLIC .)

set(SOURCE
        string_manipulation.cpp
        logging.cpp)

set(INCLUDE
        string_manipulation.h
        logging.h)

add_library(Utils STATIC ${SOURCE} ${INCLUDE})

target_include_directories(Vulkan_helpers PUBLIC .)