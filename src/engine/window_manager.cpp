#include "controller.h"
#include "window_manager.h"

#include "camera_system.h"

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

void FWindowManager::KeyboardKeyPressedOrReleased(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods)
{
    switch (Key)
    {
        case GLFW_KEY_ESCAPE:
        {
            if (Action == GLFW_PRESS) {
                glfwSetWindowShouldClose(Window, GLFW_TRUE);
            }
            break;
        }
        case GLFW_KEY_U:
        {
            if (Action == GLFW_PRESS)
            {
                auto& Context = GetContext();
            }
            break;
        }
    }
}

void FWindowManager::MouseButtonPressedOrReleased(GLFWwindow* Window, int Button, int Action, int Mods)
{
    switch (Button)
    {
        case GLFW_MOUSE_BUTTON_RIGHT:
        {
            switch (Action)
            {
                case GLFW_PRESS:
                {
                    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    CONTROLLER()->EnableCameraControlMode();
                    CONTROLLER()->ToggleFirstUpdateSinceRMB();
                    break;
                }
                case GLFW_RELEASE:
                {
                    CONTROLLER()->DisableCameraControlMode();
                    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    break;
                }
            }
            break;
        }
        case GLFW_MOUSE_BUTTON_LEFT:
        {
            switch (Action)
            {
                case GLFW_PRESS:
                {

                }
            }
        }
    }
}

void FWindowManager::MouseMoved(GLFWwindow* Window, double XPos, double YPos)
{
    CONTROLLER()->SetCameraDirection(XPos, YPos);
}

void FWindowManager::FramebufferResizeCallback(GLFWwindow* window, int Width, int Height)
{
    CONTROLLER()->Render->SetSize(Width, Height);
    CAMERA_SYSTEM()->SetAspectRatio(CONTROLLER()->Camera, float(Width) / float(Height));
}

