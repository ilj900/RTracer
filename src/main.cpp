#include "entity.h"
#include "GLFW/glfw3.h"

#include "director.h"

#include <iostream>
#include <string>

const uint32_t WINDOW_WIDTH = 1920;
const uint32_t WINDOW_HEIGHT = 1080;
const std::string WINDOW_NAME = "RTracer";

int main()
{
    glfwInit();
    GLFWwindow* Window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME.c_str(), nullptr, nullptr);

    auto Director = VKH::GetDirector();
    Director->Init();

    while (!glfwWindowShouldClose(Window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(Window);
    glfwTerminate();
    return 0;
}