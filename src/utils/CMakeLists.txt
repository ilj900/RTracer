set(INCLUDE
        logging.h
        string_manipulation.h
        utils.h)

set(SOURCE
        logging.cpp
        string_manipulation.cpp
        utils.cpp)

add_library(Utils STATIC ${SOURCE} ${INCLUDE})

target_include_directories(Vulkan_helpers PUBLIC .)