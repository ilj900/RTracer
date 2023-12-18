set(INCLUDE
        string_manipulation.h
        logging.h)

set(SOURCE
        string_manipulation.cpp
        logging.cpp)

add_library(Utils STATIC ${SOURCE} ${INCLUDE})

target_include_directories(Renderer PUBLIC .)
target_include_directories(Vulkan_helpers PUBLIC .)