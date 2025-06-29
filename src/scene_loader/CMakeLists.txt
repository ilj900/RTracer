set(INCLUDE
        scene_loader.h)

set(SOURCE
        scene_loader.cpp)

add_library(SceneLoader ${SOURCE} ${INCLUDE})

target_include_directories(RTracer PUBLIC .)
target_include_directories(Test PUBLIC .)

target_link_libraries(SceneLoader Renderer tiny_gltf)