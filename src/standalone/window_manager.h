#pragma once

#include "GLFW/glfw3.h"

#include "render.h"

#include <string>

class FApplication;
class FController;

class FWindowManager
{
public:
    FWindowManager(uint32_t& WidthIn, uint32_t& HeightIn, bool bFullscreenIn, FApplication* ApplicationIn, const std::string& NameIn);
    ~FWindowManager();
    bool ShouldClose();
    int GetWidth();
    int GetHeight();
    FVector2 GetSize2D();
	void PollEvents();
    GLFWwindow* GetWindow();
	std::vector<std::string> GetRequiredDeviceExtensions();
	std::function<VkSurfaceKHR(VkInstance)> CreateSurfaceFunctor = nullptr;
	void SetController(FController* ControllerIn);

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
	FController* Controller = nullptr;
};
