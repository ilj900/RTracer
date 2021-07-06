cmake_minimum_required(VERSION 3.13)

project(RTracer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(glfw)

include_directories(ECS
                    glfw/include
                    ${Vulkan}/Include)

set(SOURCE src/main.cpp)

set(INCLUDE ECS/entity.h)

add_executable(RTracer ${SOURCE} ${INCLUDE})
target_link_libraries(RTracer glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)