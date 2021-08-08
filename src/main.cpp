#include "entity.h"
#include "GLFW/glfw3.h"
#include "context.h"

#include <string>

const uint32_t WINDOW_WIDTH = 1920;
const uint32_t WINDOW_HEIGHT = 1080;
const std::string WINDOW_NAME = "RTracer";

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* Window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME.c_str(), nullptr, nullptr);

    V::FContext Context(Window);
    Context.Init();

    while (!glfwWindowShouldClose(Window))
    {
        Context.DrawFrame();
        glfwPollEvents();
    }

    Context.CleanUp();

    glfwDestroyWindow(Window);
    glfwTerminate();
    return 0;
}