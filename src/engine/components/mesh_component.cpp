#include "mesh_component.h"

#include "vulkan/vulkan.h"

/// Implementation of hash function
/// I'm not sure this is a good hash function
size_t std::hash<ECS::COMPONENTS::FVertexComponent>::operator()(ECS::COMPONENTS::FVertexComponent const& Vertex) const
{
    return ((std::hash<FVector3>{}(Vertex.Position) ^
             (std::hash<FVector3>{}(Vertex.Normal) << 1)) >> 1) ^
           (std::hash<FVector2>{}(Vertex.TexCoord) << 1);
}

namespace ECS
{
    namespace COMPONENTS
    {
        FVertexComponent::FVertexComponent()
        {
            Position = {0.f, 0.f, 0.f};
            Normal = {0.f, 0.f, 0.f};
            TexCoord = {0.f, 0.f};
        };

        FVertexComponent::FVertexComponent(float PosX, float PosY, float PosZ, float NormX, float NormY, float NormZ, float TexU, float TexV)
        {
            Position = {PosX, PosY, PosZ};
            Normal = {NormX, NormY, NormZ};
            TexCoord = {TexU, TexV};
        }

        bool FVertexComponent::operator==(const FVertexComponent& Other) const
        {
            return Position == Other.Position && TexCoord == Other.TexCoord;
        }

        FVertexComponent& FVertexComponent::operator=(const FVertexComponent& Other)
        {
            Position.X = Other.Position.X;
            Position.Y = Other.Position.Y;
            Position.Z = Other.Position.Z;
            Normal.X = Other.Normal.X;
            Normal.Y = Other.Normal.Y;
            Normal.Z = Other.Normal.Z;
            TexCoord.X = Other.TexCoord.X;
            TexCoord.Y = Other.TexCoord.Y;
            return *this;
        }
    }
}