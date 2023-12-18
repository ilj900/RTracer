#pragma once

#include "GLFW/glfw3.h"

#include "render.h"

class FController
{
public:
    FController();
    void SetWindow(GLFWwindow* WindowIn);
    void SetRender(std::shared_ptr<FRender> RenderIn);
    void UpdateCallbacks();
    void Update(float Time);

public:
    /// Data
    GLFWwindow* Window = nullptr;
    std::shared_ptr<FRender> Render = nullptr;

    double XPrevious = 0.;
    double YPrevious = 0.;
    double XCurrent = 0.;
    double YCurrent = 0.;

    bool Active = false;
    bool FirstUpdateSinceRMB = false;

    ECS::FEntity Camera;
};

void KeyboardKeyPressedOrReleased(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods);
void MouseButtonPressedOrReleased(GLFWwindow* Window, int Button, int Action, int Mods);
void MouseMoved(GLFWwindow* Window, double XPos, double YPos);
void FramebufferResizeCallback(GLFWwindow* window, int Width, int Height);
