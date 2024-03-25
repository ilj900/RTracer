#include "controller.h"
#include "components/camera_component.h"
#include "components/device_camera_component.h"
#include "systems/camera_system.h"

#include <iostream>
#include <utility>

FController* FController::Controller = nullptr;

FController::FController()
{
    auto& Coordinator = ECS::GetCoordinator();

    Camera = Coordinator.CreateEntity();
    Coordinator.AddComponent<ECS::COMPONENTS::FCameraComponent>(Camera, ECS::COMPONENTS::FCameraComponent());
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera, {});
}

void FController::SetWindow(GLFWwindow* WindowIn)
{
    Window = WindowIn;
}

void FController::SetRender(std::shared_ptr<FRender> RenderIn)
{
    Render = std::move(RenderIn);
}

void FController::SetCameraDirection(float XPos, float YPos)
{
    XCurrent = XPos;
    YCurrent = YPos;
}

void FController::EnableCameraControlMode()
{
    CameraControlMode = true;
}

void FController::DisableCameraControlMode()
{
    CameraControlMode = false;
}

void FController::ToggleFirstUpdateSinceRMB()
{
    FirstUpdateSinceRMB = true;
}

void FController::Update(float Time)
{
    static float CameraMovementSpeed = 1.4f;
    static float CameraRotationSpeed = 0.4f;

    if (!CameraControlMode)
        return;

    if (glfwGetKey(Window, GLFW_KEY_W))
    {
        CAMERA_SYSTEM()->MoveCameraForward(Camera, Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_S))
    {
        CAMERA_SYSTEM()->MoveCameraForward(Camera, -Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_A))
    {
        CAMERA_SYSTEM()->MoveCameraRight(Camera, -Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_D))
    {
        CAMERA_SYSTEM()->MoveCameraRight(Camera, Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_Z))
    {
        CAMERA_SYSTEM()->MoveCameraUpward(Camera, Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_C))
    {
        CAMERA_SYSTEM()->MoveCameraUpward(Camera, -Time * CameraMovementSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_Q))
    {
        CAMERA_SYSTEM()->Roll(Camera, -Time * CameraRotationSpeed);
    }
    if (glfwGetKey(Window, GLFW_KEY_E))
    {
        CAMERA_SYSTEM()->Roll(Camera, Time * CameraRotationSpeed);
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
            CAMERA_SYSTEM()->LookRight(Camera, float(-XDelta * Sensitivity));
            CAMERA_SYSTEM()->LookUp(Camera, float(-YDelta * Sensitivity));
        }
    }
}

FController* GetController()
{
    if (nullptr == FController::Controller)
    {
        FController::Controller = new FController();
    }

    return FController::Controller;
}