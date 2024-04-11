#pragma once

#include <string>

#include "vulkan/vulkan.h"

namespace V {
    void SetName(VkDevice Device, uint64_t Object, VkObjectType ObjectType, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkBuffer Buffer, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkBufferView BufferView, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkCommandBuffer CommandBuffer, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkCommandPool CommandPool, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkDescriptorPool DescriptorPool, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkDescriptorSet DescriptorSet, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkDescriptorSetLayout DescriptorSetLayout, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkDeviceMemory DeviceMemory, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkFramebuffer Framebuffer, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkImage Image, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkImageView ImageView, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkPipeline Pipeline, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkPipelineLayout PipelineLayout, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkQueryPool QueryPool, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkQueue Queue, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkRenderPass RenderPass, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkSampler Sampler, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkSemaphore Semaphore, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkShaderModule ShaderModule, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkSwapchainKHR SwapchainKHR, const std::string &Name, uint32_t Index = UINT32_MAX);

    void SetName(VkDevice Device, VkAccelerationStructureKHR AccelerationStructure, const std::string& Name, uint32_t Index = UINT32_MAX);
}