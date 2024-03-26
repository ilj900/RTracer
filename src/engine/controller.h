#pragma once

#include "GLFW/glfw3.h"

#include "render.h"

class FController
{
public:
    FController();
    void SetWindow(GLFWwindow* WindowIn);
    void Update(float Time);
    void SetCameraDirection(float XPos, float YPos);
    void EnableCameraControlMode();
    void DisableCameraControlMode();
    void ToggleFirstUpdateSinceRMB();

public:
    /// Data
    GLFWwindow* Window = nullptr;

    double XPrevious = 0.;
    double YPrevious = 0.;
    double XCurrent = 0.;
    double YCurrent = 0.;

    bool CameraControlMode = false;
    bool FirstUpdateSinceRMB = false;

    ECS::FEntity Camera;

    static FController* Controller;
};

FController* GetController();

#define CONTROLLER() GetController()
