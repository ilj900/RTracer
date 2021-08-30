#pragma once

#include "vulkan/vulkan.h"

#include "maths.h"
#include "resource_allocation.h"

#include <vector>
#include <array>
#include <memory>

struct FVertex {
    static VkVertexInputBindingDescription GetBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();

    bool operator==(const FVertex& Other) const;

    FVector3 Pos;
    FVector3 Color;
    FVector2 TexCoord;

};

template<>
struct std::hash<FVertex>
{
    size_t operator()(FVertex const& Vertex) const;
};

class FModel
{
public:
    FModel(const std::string &Path, VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator);

    std::vector<FVertex> Vertices;
    std::vector<uint32_t> Indices;

    FBuffer VertexBuffer;
    FBuffer IndexBuffer;
    FBuffer StagingBuffer;
};