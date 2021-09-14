#include "controller.h"

FController::FController(GLFWwindow* Window):
Window(Window)
{
    glfwSetWindowUserPointer(Window, this);
    glfwSetKeyCallback(Window, ProcessKeyInput);
    glfwSetCursorPosCallback(Window, ProcessMouseInput);
    glfwSetMouseButtonCallback(Window, ProcessMouseButtonInput);
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
    static float Speed = 0.4f;
    Time *= Speed;
    if (KeyMap[GLFW_KEY_W])
    {
        Camera.WS(Time);
    }
    if (KeyMap[GLFW_KEY_S])
    {
        Camera.WS(-Time);
    }
    if (KeyMap[GLFW_KEY_A])
    {
        Camera.AD(-Time);
    }
    if (KeyMap[GLFW_KEY_D])
    {
        Camera.AD(Time);
    }
    if (KeyMap[GLFW_KEY_Z])
    {
        Camera.ZC(Time);
    }
    if (KeyMap[GLFW_KEY_C])
    {
        Camera.ZC(-Time);
    }
    if (KeyMap[GLFW_KEY_Q])
    {
        Camera.QE(-Time);
    }
    if (KeyMap[GLFW_KEY_E])
    {
        Camera.QE(Time);
    }
    if (KeyMap[GLFW_MOUSE_BUTTON_LEFT])
    {
        double XDelta = XCurrent - XPrevious;
        double YDelta = YCurrent - YPrevious;
        XPrevious = XCurrent;
        YPrevious = YCurrent;
        static double Sensetivity = 0.001;
        Camera.MLMR(-XDelta * Sensetivity);
        Camera.MUMD(-YDelta * Sensetivity);
    }
}