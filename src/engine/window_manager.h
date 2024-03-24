#pragma once

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "maths.h"

#include <string>

class FWindowManager
{
public:
    explicit FWindowManager(int WidthIn, int HeightIn, bool bFullscreenIn, const std::string& NameIn);
    ~FWindowManager();
    bool ShouldClose();
    int GetWidth();
    int GetHeight();
    FVector2 GetSize2D();
    GLFWwindow* GetWindow();
    static FWindowManager* WindowManager;
private:
    GLFWwindow* Window;
    int Width = 1920;
    int Height = 1080;
    std::string Name;
    bool bFullscreen = false;
};

FWindowManager* GetWindowManager(int WidthIn = 1920, int HeightIn = 1080, bool bFullscreenIn = false, const std::string& NameIn = "Default name");

void KeyboardKeyPressedOrReleased(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods);
void MouseButtonPressedOrReleased(GLFWwindow* Window, int Button, int Action, int Mods);
void MouseMoved(GLFWwindow* Window, double XPos, double YPos);
void FramebufferResizeCallback(GLFWwindow* window, int Width, int Height);

#define WINDOW_MANAGER() GetWindowManager()
#define INIT_WINDOW_MANAGER(WidthIn, HeightIn, bFullscreenIn, NameIn) GetWindowManager(WidthIn, HeightIn, bFullscreenIn, NameIn)