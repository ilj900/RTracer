#pragma once

#include "GLFW/glfw3.h"

#include "coordinator.h"

class FController
{
public:
    FController();
    void SetWindow(GLFWwindow* Window);
    void UpdateCallbacks();
    void Update(float Time);

public:
    /// Data
    GLFWwindow* Window = nullptr;

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
