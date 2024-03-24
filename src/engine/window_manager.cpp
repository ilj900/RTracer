#include "window_manager.h"

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

FWindowManager* GetWindowManager(int WidthIn, int HeightIn, bool bFullscreenIn, const std::string& NameIn)
{
    if (nullptr == FWindowManager::WindowManager)
    {
        FWindowManager::WindowManager = new FWindowManager(WidthIn, HeightIn, bFullscreenIn, NameIn);
    }

    return FWindowManager::WindowManager;
}
