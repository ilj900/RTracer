#pragma once

#include "maths.h"
#include "common_structures.h"

#include <vector>

struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

namespace ECS
{
    namespace COMPONENTS
    {
        /// Not sure this is the right place to define this structure
        struct FVertexComponent : FVertex
        {
            FVertexComponent();
            FVertexComponent(float PosX, float PosY, float PosZ, float NormX, float NormY, float NormZ, float TexU, float TexV);

            bool operator==(const FVertexComponent& Other) const;
            FVertexComponent& operator=(const FVertexComponent& Other);
        };

        struct FMeshComponent
        {
            std::vector<FVertexComponent> Vertices{};
            std::vector<uint32_t> Indices{};
			float Area = 0.f;
            bool Indexed = false;
        };
    }
}

/// FVertex hash function, to we could use them in maps/sets
template<>
struct std::hash<ECS::COMPONENTS::FVertexComponent>
{
    size_t operator()(ECS::COMPONENTS::FVertexComponent const& Vertex) const;
};
