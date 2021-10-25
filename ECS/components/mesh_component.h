#pragma once

#include "maths.h"

#include <vector>

struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

struct FVertex {
    FVertex() = default;
    FVertex(float PosX, float PosY, float PosZ, float NormX, float NormY, float NormZ, float TexU, float TexV);

    bool operator==(const FVertex& Other) const;
    FVertex& operator=(const FVertex& Other);

    static VkVertexInputBindingDescription FVertex::GetBindingDescription();
    static std::vector<VkVertexInputAttributeDescription> FVertex::GetAttributeDescriptions();

    FVector3 Position = {0.f, 0.f, 0.f};
    FVector3 Normal = {0.f, 0.f, 0.f};
    FVector2 TexCoord = {0.f, 0.f};

};

template<>
struct std::hash<FVertex>
{
    size_t operator()(FVertex const& Vertex) const;
};

namespace ECS
{
    namespace COMPONENTS
    {
        struct FMeshComponent
        {
            std::vector<FVertex> Vertices{};
            std::vector<uint32_t> Indices{};
            bool Indexed = false;
        };
    }
}