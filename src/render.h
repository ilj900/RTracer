#pragma once

#include "GLFW/glfw3.h"

#include "image.h"

#include "maths.h"

#include "entities/entity.h"

#include <string>
#include <vector>

enum MeshType {Tetrahedron, Hexahedron, Icosahedron, Model};

class FRender
{
public:
    FRender();
    ~FRender();

    int Render();
    int Cleanup();

    int LoadDataToGPU();
    int LoadModels(const std::string& Path);
    int AddMesh(const FVector3& Color, const FVector3& Position, MeshType Type, const std::string& Path, uint32_t RenderableMask);

    GLFWwindow* Window;

    const uint32_t WINDOW_WIDTH = 1920;
    const uint32_t WINDOW_HEIGHT = 1080;
    const std::string WINDOW_NAME = "RTracer";

    ImagePtr UtilityImageR32 = nullptr;
    ImagePtr UtilityImageR8G8B8A8_SRGB = nullptr;

    std::vector<ECS::FEntity> Models;
};
