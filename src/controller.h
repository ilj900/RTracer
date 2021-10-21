#pragma once

#include "GLFW/glfw3.h"

#include "coordinator.h"

#include <unordered_map>

class FController
{
public:
    FController(GLFWwindow* Window);
    void Update(float Time);
    void SetPressed(uint32_t Key);
    void SetReleased(uint32_t Key);

public:
    /// Data
    GLFWwindow* Window = nullptr;
    std::unordered_map<uint32_t , bool> KeyMap =
            {
                    {GLFW_KEY_W, false},
                    {GLFW_KEY_S, false},
                    {GLFW_KEY_A, false},
                    {GLFW_KEY_D, false},
                    {GLFW_KEY_Z, false},
                    {GLFW_KEY_C, false},
                    {GLFW_KEY_Q, false},
                    {GLFW_KEY_E, false},
                    {GLFW_KEY_W, false},
                    {GLFW_KEY_ESCAPE, false},
            };
    double XPrevious = 0.;
    double YPrevious = 0.;
    double XCurrent = 0.;
    double YCurrent = 0.;

    ECS::FEntity Camera;
};

void ProcessKeyInput(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods);
void ProcessMouseInput(GLFWwindow* Window, double XPos, double YPos);
