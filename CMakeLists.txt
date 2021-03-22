cmake_minimum_required(VERSION 3.13)

project(cmake_template_app)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

set(SOURCE src/main.cpp)

set(INCLUDE include/main.h)

add_executable(cmake_template_app ${SOURCE} ${INCLUDE})