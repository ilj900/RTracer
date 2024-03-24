#pragma once

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include <string>

class FWindowManager
{
public:
    explicit FWindowManager(int WidthIn, int HeightIn, bool bFullscreenIn, const std::string& NameIn);
    ~FWindowManager();
    bool ShouldClose();

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

#define WINDOW_MANAGER() GetWindowManager()
#define INIT_WINDOW_MANAGER(WidthIn, HeightIn, bFullscreenIn, NameIn) GetWindowManager(WidthIn, HeightIn, bFullscreenIn, NameIn)