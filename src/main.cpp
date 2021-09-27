#include "coordinator.h"
#include "components/camera_component.h"
#include "systems/camera_system.h"
#include "GLFW/glfw3.h"
#include "context.h"

#include <chrono>
#include <string>

const uint32_t WINDOW_WIDTH = 1920;
const uint32_t WINDOW_HEIGHT = 1080;
const std::string WINDOW_NAME = "RTracer";

void InitECS(ECS::FCoordinator& Coordinator)
{
    Coordinator.Init();

    Coordinator.RegisterComponent<ECS::COMPONENTS::FCameraComponent>();
    auto CameraSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FCameraSystem>();

    ECS::FSignature CameraSystemSignature;
    CameraSystemSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FCameraComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FCameraSystem>(CameraSystemSignature);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    GLFWwindow* Window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME.c_str(), nullptr, nullptr);

    auto& Coordinator = ECS::GetCoordinator();
    InitECS(Coordinator);

    ECS::FEntity MainCamera = Coordinator.CreateEntity();

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