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
                    math)

set(SOURCE  src/main.cpp
            vulkan_helpers/context.cpp
            math/maths.cpp)

set(INCLUDE ECS/entity.h
            vulkan_helpers/context.h
            include/stb_image.h
            include/tiny_obj_loader.h
            math/maths.h)

add_executable(RTracer ${SOURCE} ${INCLUDE})

target_link_libraries(RTracer glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)