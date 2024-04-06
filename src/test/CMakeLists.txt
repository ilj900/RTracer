set(INCLUDE
        test.h)

set(SOURCE
        test.cpp)

add_executable(Test ${SOURCE} ${INCLUDE})

target_link_libraries(Test PRIVATE Catch2::Catch2WithMain)