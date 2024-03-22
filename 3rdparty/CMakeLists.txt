add_subdirectory(tinyexr)
add_subdirectory(stb_image)
add_subdirectory(tiny_obj_loader)
add_subdirectory(glfw)

set(SOURCE
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_vulkan.cpp
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp)

add_library(imgui STATIC ${SOURCE})

target_link_libraries(imgui Vulkan::Vulkan glfw)

target_include_directories(imgui PUBLIC imgui imgui/backends ImGuiLegitProfiler)
target_include_directories(RTracer PUBLIC .)
