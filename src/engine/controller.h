#pragma once

#include "GLFW/glfw3.h"

#include "render.h"

class FController
{
public:
    FController();
    void SetWindow(GLFWwindow* WindowIn);
    void SetRender(std::shared_ptr<FRender> RenderIn);
    void Update(float Time);

public:
    /// Data
    GLFWwindow* Window = nullptr;
    std::shared_ptr<FRender> Render = nullptr;

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
