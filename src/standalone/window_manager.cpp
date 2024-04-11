#include "application.h"
#include "controller.h"
#include "window_manager.h"

FWindowManager::FWindowManager(int WidthIn, int HeightIn, bool bFullscreenIn, FApplication* ApplicationIn, const std::string &NameIn) : Width(WidthIn), Height(HeightIn), bFullscreen(bFullscreenIn), Application(ApplicationIn), Name(NameIn)
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
	glfwSetWindowUserPointer(Window, this);

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


void FWindowManager::SetController(FController* ControllerIn)
{
	Controller = ControllerIn;
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
            }
            break;
        }
    }
}

void FWindowManager::MouseButtonPressedOrReleased(GLFWwindow* Window, int Button, int Action, int Mods)
{
	auto* WindowManager = (FWindowManager*)glfwGetWindowUserPointer(Window);

    switch (Button)
    {
        case GLFW_MOUSE_BUTTON_RIGHT:
        {
            switch (Action)
            {
                case GLFW_PRESS:
                {
                    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    WindowManager->Controller->EnableCameraControlMode();
					WindowManager->Controller->ToggleFirstUpdateSinceRMB();
                    break;
                }
                case GLFW_RELEASE:
                {
					WindowManager->Controller->DisableCameraControlMode();
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
	auto* WindowManager = (FWindowManager*)glfwGetWindowUserPointer(Window);

	WindowManager->Controller->SetCameraDirection(XPos, YPos);
}

void FWindowManager::FramebufferResizeCallback(GLFWwindow* Window, int Width, int Height)
{
	FWindowManager* WindowManager = (FWindowManager*)glfwGetWindowUserPointer(Window);
	WindowManager->Application->SetSwapchainWasResized(Width, Height);
    RENDER()->SetSize(Width, Height);
}

