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

        VkVertexInputBindingDescription FVertexComponent::GetBindingDescription()
        {
            VkVertexInputBindingDescription BindingDescription{};
            BindingDescription.binding = 0;
            BindingDescription.stride = sizeof(FVertexComponent);
            BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return BindingDescription;
        }

        std::vector<VkVertexInputAttributeDescription> FVertexComponent::GetAttributeDescriptions()
        {
            std::vector<VkVertexInputAttributeDescription> AttributeDescription{};

            VkVertexInputAttributeDescription VertexInputAttributeDescription;

            VertexInputAttributeDescription.location = 0;
            VertexInputAttributeDescription.binding = 0;
            VertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
            VertexInputAttributeDescription.offset = offsetof(FVertexComponent, Position);
            AttributeDescription.push_back(VertexInputAttributeDescription);

            VertexInputAttributeDescription.location = 1;
            VertexInputAttributeDescription.binding = 0;
            VertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
            VertexInputAttributeDescription.offset = offsetof(FVertexComponent, Normal);
            AttributeDescription.push_back(VertexInputAttributeDescription);

            VertexInputAttributeDescription.location = 2;
            VertexInputAttributeDescription.binding = 0;
            VertexInputAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
            VertexInputAttributeDescription.offset = offsetof(FVertexComponent, TexCoord);
            AttributeDescription.push_back(VertexInputAttributeDescription);

            return AttributeDescription;
        }
    }
}