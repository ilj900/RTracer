#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

enum class eShaderType {VERTEX, FRAGMENT, COMPUTE, RAYGEN, RAYMISS, RAYCLOSEHIT};

class FPipeline
{
public:
    FPipeline() = default;

    void AddShader(const std::string& Path, eShaderType ShaderType);
    void AddVertexInputBindingDescription(const VkVertexInputBindingDescription& VertexInputBindingDescription);
    void AddVertexInputAttributeDescription(const VkVertexInputAttributeDescription& VertexInputAttributeDescription);
    void SetMSAA(VkSampleCountFlagBits SampleCountFlagBits);
    void SetExtent2D(VkExtent2D Extent2D);
    void AddDescriptorSetLayout(VkDescriptorSetLayout DescriptorSetLayout);
    void SetWidth(uint32_t Wdth);
    void SetHeight(uint32_t Hght);
    VkPipelineLayout GetPipelineLayout();
    VkPipeline GetPipeline();

    VkPipeline CreateGraphicsPipeline(VkDevice Device, VkRenderPass RenderPass);
    VkPipeline CreateRayTracingPipeline(VkDevice LogicalDevice);

    void Delete();

    ~FPipeline();

private:
    VkPipelineLayout PipelineLayout;
    VkPipeline Pipeline;
    VkDevice LogicalDevice;
    VkRenderPass RenderPass;

    VkShaderModule CreateShaderFromFile(const std::string& FileName);

    struct FShaderModule
    {
        VkShaderModule ShaderModule;
        eShaderType ShaderType;
    };

    std::vector<FShaderModule> ShaderModules;

    std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptionVector{};
    std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptionVector{};

    uint32_t Width = 1920;
    uint32_t Height = 1080;

    VkSampleCountFlagBits MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    VkExtent2D Extent = {1920, 1080};

    std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
};