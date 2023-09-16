#include "controller.h"
#include "components/camera_component.h"
#include "components/device_camera_component.h"
#include "systems/camera_system.h"
#include "systems/renderable_system.h"
#include "vk_context.h"

#include <iostream>
#include <map>
#include <utility>

FController::FController()
{
    auto& Coordinator = ECS::GetCoordinator();

    Camera = Coordinator.CreateEntity();
    Coordinator.AddComponent<ECS::COMPONENTS::FCameraComponent>(Camera, ECS::COMPONENTS::FCameraComponent());
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera, {});
}

void FController::SetWindow(GLFWwindow* Window)
{
    this->Window = Window;
}

void FController::SetRender(std::shared_ptr<FRender> Render)
{
    this->Render = std::move(Render);
}

void FramebufferResizeCallback(GLFWwindow* Window, int Width, int Height) {
    auto Controller = static_cast<FController*>(glfwGetWindowUserPointer(Window));
    Controller->Render->SetSize(Width, Height);
    CAMERA_SYSTEM()->SetAspectRatio(Controller->Camera, float(Width) / float(Height));
}

void FController::UpdateCallbacks()
{
    glfwSetWindowUserPointer(Window, this);
    glfwSetKeyCallback(Window, KeyboardKeyPressedOrReleased);
    glfwSetCursorPosCallback(Window, MouseMoved);
    glfwSetMouseButtonCallback(Window, MouseButtonPressedOrReleased);
    glfwSetFramebufferSizeCallback(Window, FramebufferResizeCallback);
}

void KeyboardKeyPressedOrReleased(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods) {
    switch (Key)
    {
        case GLFW_KEY_ESCAPE:
        {
            if (Action == GLFW_PRESS) {
                glfwSetWindowShouldClose(Window, GLFW_TRUE);
            }
            break;
        }
        case GLFW_KEY_U:
        {
            if (Action == GLFW_PRESS) {
                auto& Context = GetContext();
//                Context.NormalsImage->Resolve(*Context.UtilityImageR8G8B8A8_SRGB);
//                Context.SaveImage(*Context.UtilityImageR8G8B8A8_SRGB);
            }
            break;
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
        case GLFW_MOUSE_BUTTON_LEFT:
        {
            switch (Action)
            {
                case GLFW_PRESS:
                {
                    auto& Context = GetContext();
                    std::vector<uint32_t> Data;

//                    Context.RenderableIndexImage->Resolve(*Context.UtilityImageR32);
//                    Context.FetchImageData(*Context.UtilityImageR32, Data);
//                    double X, Y;
//                    glfwGetCursorPos(Window, &X, &Y);
//
//                    auto Index = (uint32_t(Y) * Context.Swapchain->GetWidth() + uint32_t(X));
//
//                    uint32_t RenderableIndex = Data[Index];
//                    auto RenderableSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FRenderableSystem>();
//                    RenderableSystem->SetSelectedByIndex(RenderableIndex);
                }
            }
        }
    }
}

void FController::Update(float Time)
{
    static float CameraMovementSpeed = 1.4f;
    static float CameraRotationSpeed = 0.4f;

    if (!Active)
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