#include "entity.h"
#include "GLFW/glfw3.h"
#include "context.h"

#include "controller.h"

#include <chrono>
#include <string>

const uint32_t WINDOW_WIDTH = 1920;
const uint32_t WINDOW_HEIGHT = 1080;
const std::string WINDOW_NAME = "RTracer";

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    GLFWwindow* Window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME.c_str(), nullptr, nullptr);

    FController Controller(Window);

    FContext Context(Window, &Controller);
    Context.Init();

    while (!glfwWindowShouldClose(Window))
    {
        static auto StartTime = std::chrono::high_resolution_clock::now();

        auto CurrentTime = std::chrono::high_resolution_clock::now();
        float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
        StartTime = CurrentTime;

        Controller.Update(Time);
        Context.Render();
        Context.Present();
        glfwPollEvents();
    }
    Context.WaitIdle();
    Context.CleanUp();

    glfwDestroyWindow(Window);
    glfwTerminate();
    return 0;
}