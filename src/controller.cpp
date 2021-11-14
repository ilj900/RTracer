#include "controller.h"
#include "components/camera_component.h"
#include "components/device_camera_component.h"
#include "systems/camera_system.h"

FController::FController(GLFWwindow* Window):
Window(Window)
{
    glfwSetWindowUserPointer(Window, this);
    glfwSetKeyCallback(Window, KeyboardKeyPressedOrReleased);
    glfwSetCursorPosCallback(Window, MouseMoved);
    glfwSetMouseButtonCallback(Window, MouseButtonPressedOrReleased);

    auto& Coordinator = ECS::GetCoordinator();
    auto CameraSystem = Coordinator.GetSystem<ECS::SYSTEMS::FCameraSystem>();

    Camera = Coordinator.CreateEntity();
    Coordinator.AddComponent<ECS::COMPONENTS::FCameraComponent>(Camera, ECS::COMPONENTS::FCameraComponent());
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera, {});
    CameraSystem->UpdateDeviceComponentData(Camera);
}

void KeyboardKeyPressedOrReleased(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods) {
    switch (Key)
    {
        case GLFW_KEY_ESCAPE:
        {
            if (Action == GLFW_PRESS) {
                glfwSetWindowShouldClose(Window, GLFW_TRUE);
            }
        }
    }
}

void MouseMoved(GLFWwindow* Window, double XPos, double YPos)
{
    auto Controller = static_cast<FController*>(glfwGetWindowUserPointer(Window));
    Controller->XCurrent = XPos;
    Controller->YCurrent = YPos;
}

void MouseButtonPressedOrReleased(GLFWwindow* Window, int Button, int Action, int Mods)
{
    switch (Button)
    {
        case GLFW_MOUSE_BUTTON_RIGHT:
        {
            switch (Action)
            {
                case GLFW_PRESS:
                {
                    auto Controller = static_cast<FController*>(glfwGetWindowUserPointer(Window));
                    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    Controller->Active = true;
                    Controller->FirstUpdateSinceRMB = true;
                    break;
                }
                case GLFW_RELEASE:
                {
                    auto Controller = static_cast<FController*>(glfwGetWindowUserPointer(Window));
                    Controller->Active = false;
                    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    break;
                }
            }
            break;
        }
    }
}

void FController::Update(float Time)
{
    static float CameraMovementSpeed = 1.4f;
    static float CameraRotationSpeed = 0.4f;

    if (!Active)
        return;

    auto CameraSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FCameraSystem>();

    if (glfwGetKey(Window, GLFW_KEY_W))
    {
        CameraSystem->MoveCameraForward(Camera, Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_S))
    {
        CameraSystem->MoveCameraForward(Camera, -Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_A))
    {
        CameraSystem->MoveCameraRight(Camera, -Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_D))
    {
        CameraSystem->MoveCameraRight(Camera, Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_Z))
    {
        CameraSystem->MoveCameraUpward(Camera, Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_C))
    {
        CameraSystem->MoveCameraUpward(Camera, -Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_Q))
    {
        CameraSystem->Roll(Camera, -Time * CameraRotationSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_E))
    {
        CameraSystem->Roll(Camera, Time * CameraRotationSpeed);
    }

    {
        double XDelta = XCurrent - XPrevious;
        double YDelta = YCurrent - YPrevious;
        XPrevious = XCurrent;
        YPrevious = YCurrent;

        /// GLFW doesn't work well with switching cursor between "disabled" and "normal"
        /// This bool and check protect from camera sudden moves
        if (FirstUpdateSinceRMB)
        {
            FirstUpdateSinceRMB = false;
        }
        else
        {
            static double Sensitivity = 0.001;
            CameraSystem->LookRight(Camera, float(-XDelta * Sensitivity));
            CameraSystem->LookUp(Camera, float(-YDelta * Sensitivity));
        }
    }
    CameraSystem->UpdateDeviceComponentData(Camera);
}