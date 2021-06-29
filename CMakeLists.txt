cmake_minimum_required(VERSION 3.13)

project(RTracer)

set(CMAKE_CXX_STANDARD 14)

include_directories(ECS)

set(SOURCE src/main.cpp)

set(INCLUDE ECS/entity.h)

add_executable(RTracer ${SOURCE} ${INCLUDE})