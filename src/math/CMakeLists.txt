target_include_directories(RTracer PUBLIC .)

set(SOURCE
        maths.cpp)

set(INCLUDE
        maths.h)

add_library(Math STATIC ${SOURCE} ${INCLUDE})