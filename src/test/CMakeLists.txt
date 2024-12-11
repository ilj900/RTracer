set(SOURCE
        test.cpp)

add_executable(Test ${SOURCE})

target_link_libraries(Test PRIVATE Catch2::Catch2WithMain PUBLIC Renderer SceneLoader stb_image)