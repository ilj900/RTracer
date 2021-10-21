#pragma once

#include "vulkan/vulkan.h"

#include "maths.h"
#include "resource_allocation.h"
#include "entities/entity.h"

#include <vector>
#include <array>
#include <memory>

struct FVertex {
    FVertex() = default;
    FVertex(float PosX, float PosY, float PosZ, float NormX, float NormY, float NormZ,float ColR, float ColG, float ColB, float TexU, float TexV);
    static VkVertexInputBindingDescription GetBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions();

    bool operator==(const FVertex& Other) const;
    FVertex& operator=(const FVertex& Other);

    FVector3 Position = {0.f, 0.f, 0.f};
    FVector3 Normal = {0.f, 0.f, 0.f};
    FVector3 Color = {0.f, 0.f, 0.f};
    FVector2 TexCoord = {0.f, 0.f};

};

template<>
struct std::hash<FVertex>
{
    size_t operator()(FVertex const& Vertex) const;
};

class FModel
{
public:
    FModel();
    FModel(const std::string &Path, VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator);

    void Draw(VkCommandBuffer CommandBuffer);
    void Bind(VkCommandBuffer CommandBuffer);

    static FModel CreateTetrahedron(VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator);
    static FModel CreateHexahedron(VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator);
    static FModel CreateIcosahedron(VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator);

    ECS::FEntity Model;

    bool Indexed = false;

    std::vector<FVertex> Vertices{};
    std::vector<uint32_t> Indices{};

    FBuffer VertexBuffer;
    FBuffer IndexBuffer;
    FBuffer StagingBuffer;
private:
    void LoadDataIntoGPU(VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator);
};