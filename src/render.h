#pragma once

#include "GLFW/glfw3.h"

#include "image.h"

#include <string>

class FRender
{
public:
    FRender();
    ~FRender();

    int Render();
    int Cleanup();

    int LoadModels(const std::string& Path);
    int AddMesh();
    GLFWwindow* CreateWindow();

    GLFWwindow* Window;

    const uint32_t WINDOW_WIDTH = 1920;
    const uint32_t WINDOW_HEIGHT = 1080;
    const std::string WINDOW_NAME = "RTracer";

    ImagePtr UtilityImageR32 = nullptr;
    ImagePtr UtilityImageR8G8B8A8_SRGB = nullptr;
};
