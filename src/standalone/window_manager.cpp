#include "application.h"
#include "controller.h"
#include "utility_functions.h"
#include "window_manager.h"

FWindowManager::FWindowManager(uint32_t& WidthIn, uint32_t& HeightIn, bool bFullscreenIn, FApplication* ApplicationIn, const std::string& NameIn) : Width(WidthIn), Height(HeightIn), bFullscreen(bFullscreenIn), Application(ApplicationIn), Name(NameIn)
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
		WidthIn = Width;
        Height = Mode->height;
		HeightIn = Height;
    }

    Window = glfwCreateWindow(Width, Height, Name.c_str(), bFullscreen ? PrimaryMonitor : nullptr, nullptr);
    glfwSetWindowPos(Window, Width / 2, Height / 2);
	glfwSetWindowUserPointer(Window, this);

    glfwSetKeyCallback(Window, KeyboardKeyPressedOrReleased);
    glfwSetCursorPosCallback(Window, MouseMoved);
    glfwSetMouseButtonCallback(Window, MouseButtonPressedOrReleased);
    glfwSetFramebufferSizeCallback(Window, FramebufferResizeCallback);

	CreateSurfaceFunctor = [this](VkInstance Instance) -> VkSurfaceKHR
	{
		VkSurfaceKHR SurfaceIn;
		VkResult Result = glfwCreateWindowSurface(Instance, Window, nullptr, &SurfaceIn);
		assert((Result == VK_SUCCESS) && "Failed to create window surface!");
		return SurfaceIn;
	};
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

void FWindowManager::PollEvents()
{
	glfwPollEvents();
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
				auto* WindowManager = (FWindowManager*)glfwGetWindowUserPointer(Window);
				SaveCamera(WindowManager->Controller->Camera, "Test");
            }
            break;
        }

		case GLFW_KEY_I:
		{
			if (Action == GLFW_PRESS)
			{
				auto* WindowManager = (FWindowManager*)glfwGetWindowUserPointer(Window);
				LoadCamera(WindowManager->Controller->Camera, "Test");
			}
			break;
		}

		case GLFW_KEY_O:
		{
			if (Action == GLFW_PRESS)
			{
				auto* WindowManager = (FWindowManager*)glfwGetWindowUserPointer(Window);
				WindowManager->Controller->Render->PrintScreenExr("EstimatedImage");
			}
			break;
		}

		case GLFW_KEY_P:
		{
			if (Action == GLFW_PRESS)
			{
				auto* WindowManager = (FWindowManager*)glfwGetWindowUserPointer(Window);
				WindowManager->Controller->Render->PrintScreenPng("EstimatedImage");
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
}
std::vector<std::string> FWindowManager::GetRequiredDeviceExtensions()
{
	uint32_t Counter = 0;
	auto ExtensionsRequiredByGLFW = glfwGetRequiredInstanceExtensions(&Counter);
	std::vector<std::string> Result;
	for (int i = 0; i < Counter; ++i)
	{
		Result.emplace_back(std::string(ExtensionsRequiredByGLFW[i]));
	}

	return Result;
}
