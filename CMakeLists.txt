cmake_minimum_required(VERSION 3.13)

project(RTracer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(glfw)

include_directories(ECS
        glfw/include
        vulkan_helpers
        ${Vulkan}/Include
        include
        math
        src)

set(SOURCE
        src/main.cpp
        src/camera.cpp
        src/controller.cpp
        vulkan_helpers/context.cpp
        vulkan_helpers/resource_allocation.cpp
        vulkan_helpers/function_loader.cpp
        vulkan_helpers/model.cpp
        vulkan_helpers/swapchain.cpp
        math/maths.cpp)

set(INCLUDE
        ECS/entity.h
        src/camera.h
        src/controller.h
        vulkan_helpers/context.h
        vulkan_helpers/resource_allocation.h
        vulkan_helpers/function_loader.h
        vulkan_helpers/model.h
        vulkan_helpers/swapchain.h
        include/stb_image.h
        include/tiny_obj_loader.h
        math/maths.h)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)