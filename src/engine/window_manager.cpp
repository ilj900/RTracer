#include "window_manager.h"

#include <iostream>

FWindowManager* FWindowManager::WindowManager = nullptr;

FWindowManager::FWindowManager(int WidthIn, int HeightIn, bool bFullscreenIn, const std::string &NameIn) : Width(WidthIn), Height(HeightIn), bFullscreen(bFullscreenIn), Name(NameIn)
{
    /// Create GLFW Window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    GLFWmonitor* PrimaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* Mode = glfwGetVideoMode(PrimaryMonitor);
    if (bFullscreen)
    {
        Width = Mode->width;
        Height = Mode->height;
    }
    Window = glfwCreateWindow(Width, Height, Name.c_str(), bFullscreen ? PrimaryMonitor : nullptr, nullptr);
    glfwSetWindowPos(Window, Width / 2, Height / 2);
    glfwSetCursorPos(Window, 0.f, 0.f);

    glfwSetKeyCallback(Window, KeyboardKeyPressedOrReleased);
    glfwSetCursorPosCallback(Window, MouseMoved);
    glfwSetMouseButtonCallback(Window, MouseButtonPressedOrReleased);
    glfwSetFramebufferSizeCallback(Window, FramebufferResizeCallback);
}

FWindowManager::~FWindowManager()
{
    glfwDestroyWindow(Window);
    glfwTerminate();
}

bool FWindowManager::ShouldClose()
{
    return glfwWindowShouldClose(Window);
}

GLFWwindow* FWindowManager::GetWindow()
{
    return Window;
}

int FWindowManager::GetWidth()
{
    return Width;
}

int FWindowManager::GetHeight()
{
    return Height;
}


FVector2 FWindowManager::GetSize2D()
{
    return FVector2(Width, Height);
}


FWindowManager* GetWindowManager(int WidthIn, int HeightIn, bool bFullscreenIn, const std::string& NameIn)
{
    if (nullptr == FWindowManager::WindowManager)
    {
        FWindowManager::WindowManager = new FWindowManager(WidthIn, HeightIn, bFullscreenIn, NameIn);
    }

    return FWindowManager::WindowManager;
}

void KeyboardKeyPressedOrReleased(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods)
{

}

void MouseButtonPressedOrReleased(GLFWwindow* Window, int Button, int Action, int Mods)
{
    std::cout << "Ara-ara!" << std::endl;
}

void MouseMoved(GLFWwindow* Window, double XPos, double YPos)
{

}

void FramebufferResizeCallback(GLFWwindow* window, int Width, int Height)
{

}

