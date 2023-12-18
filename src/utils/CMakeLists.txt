set(INCLUDE
        logging.h
        string_manipulation.h)

set(SOURCE
        logging.cpp
        string_manipulation.cpp)

add_library(Utils STATIC ${SOURCE} ${INCLUDE})

target_include_directories(Renderer PUBLIC .)
target_include_directories(Vulkan_helpers PUBLIC .)