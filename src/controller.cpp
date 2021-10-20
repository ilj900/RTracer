#include "controller.h"
#include "components/camera_component.h"
#include "components/device_camera_component.h"
#include "systems/camera_system.h"

FController::FController(GLFWwindow* Window):
Window(Window)
{
    glfwSetWindowUserPointer(Window, this);
    glfwSetKeyCallback(Window, ProcessKeyInput);
    glfwSetCursorPosCallback(Window, ProcessMouseInput);
    glfwSetMouseButtonCallback(Window, ProcessMouseButtonInput);

    auto& Coordinator = ECS::GetCoordinator();
    Camera = Coordinator.CreateEntity();
    Coordinator.AddComponent<ECS::COMPONENTS::FCameraComponent>(Camera, ECS::COMPONENTS::FCameraComponent());
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera, {});
}

void ProcessMouseButtonInput(GLFWwindow* Window, int Button, int Action, int Mods)
{
    FController* Controller = static_cast<FController*>(glfwGetWindowUserPointer(Window));
    if (Action == GLFW_PRESS)
    {
        switch (Button)
        {
            case GLFW_MOUSE_BUTTON_LEFT:
                Controller->SetPressed(Button);
                Controller->XPrevious = Controller->XCurrent;
                Controller->YPrevious = Controller->YCurrent;
                break;
        }
    }
    if (Action == GLFW_RELEASE)
    {
        switch (Button)
        {
            case GLFW_MOUSE_BUTTON_LEFT:
                Controller->SetReleased(Button);
                break;
        }
    }
}

void FController::ToggleKey(uint32_t Key)
{
    if (KeyMap.find(Key) == KeyMap.end())
    {
        return;
    }
    KeyMap[Key] = !KeyMap[Key];
}

void FController::SetPressed(uint32_t Key)
{
    if (KeyMap.find(Key) == KeyMap.end())
    {
        return;
    }
    KeyMap[Key] = true;
}

void FController::SetReleased(uint32_t Key)
{
    if (KeyMap.find(Key) == KeyMap.end())
    {
        return;
    }
    KeyMap[Key] = false;
}

void ProcessKeyInput(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods)
{
    FController* Controller = static_cast<FController*>(glfwGetWindowUserPointer(Window));
    if (Action == GLFW_PRESS)
    {
        Controller->SetPressed(Key);
    }
    if (Action == GLFW_RELEASE)
    {
        Controller->SetReleased(Key);
    }
}

void ProcessMouseInput(GLFWwindow* Window, double XPos, double YPos)
{
    FController* Controller = static_cast<FController*>(glfwGetWindowUserPointer(Window));
    Controller->XCurrent = XPos;
    Controller->YCurrent = YPos;
}

void FController::Update(float Time)
{
    static float CameraMovementSpeed = 1.4f;
    static float CameraRotationSpeed = 0.4f;

    auto CameraSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FCameraSystem>();
    if (KeyMap[GLFW_KEY_W])
    {
        CameraSystem->MoveCameraForward(Camera, Time * CameraMovementSpeed);
    }
    if (KeyMap[GLFW_KEY_S])
    {
        CameraSystem->MoveCameraForward(Camera, -Time * CameraMovementSpeed);
    }
    if (KeyMap[GLFW_KEY_A])
    {
        CameraSystem->MoveCameraRight(Camera, -Time * CameraMovementSpeed);
    }
    if (KeyMap[GLFW_KEY_D])
    {
        CameraSystem->MoveCameraRight(Camera, Time * CameraMovementSpeed);
    }
    if (KeyMap[GLFW_KEY_Z])
    {
        CameraSystem->MoveCameraUpward(Camera, Time * CameraMovementSpeed);
    }
    if (KeyMap[GLFW_KEY_C])
    {
        CameraSystem->MoveCameraUpward(Camera, -Time * CameraMovementSpeed);
    }
    if (KeyMap[GLFW_KEY_Q])
    {
        CameraSystem->Roll(Camera, -Time * CameraRotationSpeed);
    }
    if (KeyMap[GLFW_KEY_E])
    {
        CameraSystem->Roll(Camera, Time * CameraRotationSpeed);
    }
    if (KeyMap[GLFW_MOUSE_BUTTON_LEFT])
    {
        double XDelta = XCurrent - XPrevious;
        double YDelta = YCurrent - YPrevious;
        XPrevious = XCurrent;
        YPrevious = YCurrent;
        static double Sensitivity = 0.001;
        CameraSystem->LookRight(Camera, float(-XDelta * Sensitivity));
        CameraSystem->LookUp(Camera, float(-YDelta * Sensitivity));
    }
    CameraSystem->UpdateDeviceComponentData(Camera);
}