#pragma once

#include "render.h"
#include "GLFW/glfw3.h"
#include "maths.h"
#include "application.h"

#include <string>

class FWindowManager
{
public:
    explicit FWindowManager(int WidthIn, int HeightIn, bool bFullscreenIn, FApplication* ApplicationIn, const std::string& NameIn);
    int Destroy();
    bool ShouldClose();
    int GetWidth();
    int GetHeight();
    FVector2 GetSize2D();
    GLFWwindow* GetWindow();

    static FWindowManager* WindowManager;

    static void KeyboardKeyPressedOrReleased(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods);
    static void MouseButtonPressedOrReleased(GLFWwindow* Window, int Button, int Action, int Mods);
    static void MouseMoved(GLFWwindow* Window, double XPos, double YPos);
    static void FramebufferResizeCallback(GLFWwindow* window, int Width, int Height);
private:
    GLFWwindow* Window;
    int Width = 1920;
    int Height = 1080;
    std::string Name;
    bool bFullscreen = false;
	FApplication* Application = nullptr;
};

FWindowManager* GetWindowManager(int WidthIn = 1920, int HeightIn = 1080, bool bFullscreenIn = false, FApplication* ApplicationIn = nullptr, const std::string& NameIn = "Default name");

#define INIT_WINDOW_MANAGER(WidthIn, HeightIn, bFullscreenIn, ApplicationIn, NameIn) GetWindowManager(WidthIn, HeightIn, bFullscreenIn, ApplicationIn, NameIn)
#define WINDOW_MANAGER() GetWindowManager()
#define WINDOW() GetWindowManager()->GetWindow()
