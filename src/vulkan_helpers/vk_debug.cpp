#include "vk_functions.h"
#include "vk_debug.h"

#include <cassert>
#include <unordered_map>

namespace V
{
    void SetName(VkDevice Device, uint64_t Object, VkObjectType ObjectType, const std::string& Name, uint32_t Index) {
#ifndef NDEBUG
        std::unordered_map<VkObjectType, std::string> TypeToPostfixMap =
        {
                {VK_OBJECT_TYPE_BUFFER, " Buffer "},
                {VK_OBJECT_TYPE_BUFFER_VIEW, " Buffer View "},
                {VK_OBJECT_TYPE_COMMAND_BUFFER, " Command Buffer "},
                {VK_OBJECT_TYPE_COMMAND_POOL, " Command Pool "},
                {VK_OBJECT_TYPE_DESCRIPTOR_POOL, " Descriptor Pool "},
                {VK_OBJECT_TYPE_DESCRIPTOR_SET, " Descriptor Set "},
                {VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, " Descriptor Set Layout "},
                {VK_OBJECT_TYPE_DEVICE_MEMORY, " Device Memory "},
                {VK_OBJECT_TYPE_FRAMEBUFFER, " Framebuffer "},
                {VK_OBJECT_TYPE_IMAGE, " Image "},
                {VK_OBJECT_TYPE_IMAGE_VIEW, " Image View "},
                {VK_OBJECT_TYPE_PIPELINE, " Pipeline "},
                {VK_OBJECT_TYPE_PIPELINE_LAYOUT, " Pipeline Layout "},
                {VK_OBJECT_TYPE_QUERY_POOL, " Query Pool "},
                {VK_OBJECT_TYPE_QUEUE, " Queue "},
                {VK_OBJECT_TYPE_RENDER_PASS, " Render Pass "},
                {VK_OBJECT_TYPE_SAMPLER, " Sampler "},
                {VK_OBJECT_TYPE_SEMAPHORE, " Semaphore "},
                {VK_OBJECT_TYPE_SHADER_MODULE, " Shader module "},
                {VK_OBJECT_TYPE_SWAPCHAIN_KHR, " Swapchain KHR "},
                {VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR, " Acceleration structure KHR "},
        };
        std::string FullName = "V::" + Name + TypeToPostfixMap[ObjectType];

		if (Index != UINT32_MAX)
		{
			FullName += std::to_string(Index);
		}

        VkDebugUtilsObjectNameInfoEXT DebugUtilsObjectNameInfo{};
        DebugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        DebugUtilsObjectNameInfo.objectType = ObjectType;
        DebugUtilsObjectNameInfo.objectHandle = Object;
        DebugUtilsObjectNameInfo.pObjectName = FullName.c_str();

        VkResult Result = vkSetDebugUtilsObjectNameEXT(Device, &DebugUtilsObjectNameInfo);
        assert((Result == VK_SUCCESS) && "Failed to give a name to a vulkan object!");
#endif
    }

    void SetName(VkDevice Device, VkBuffer Buffer, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(Buffer), VK_OBJECT_TYPE_BUFFER, Name, Index); }
    void SetName(VkDevice Device, VkBufferView BufferView, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(BufferView), VK_OBJECT_TYPE_BUFFER_VIEW, Name, Index); }
    void SetName(VkDevice Device, VkCommandBuffer CommandBuffer, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(CommandBuffer), VK_OBJECT_TYPE_COMMAND_BUFFER, Name, Index); }
    void SetName(VkDevice Device, VkCommandPool CommandPool, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(CommandPool), VK_OBJECT_TYPE_COMMAND_POOL, Name, Index); }
    void SetName(VkDevice Device, VkDescriptorPool DescriptorPool, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(DescriptorPool), VK_OBJECT_TYPE_DESCRIPTOR_POOL, Name, Index); }
    void SetName(VkDevice Device, VkDescriptorSet DescriptorSet, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(DescriptorSet), VK_OBJECT_TYPE_DESCRIPTOR_SET, Name, Index); }
    void SetName(VkDevice Device, VkDescriptorSetLayout DescriptorSetLayout, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(DescriptorSetLayout), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, Name, Index); }
    void SetName(VkDevice Device, VkDeviceMemory DeviceMemory, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(DeviceMemory), VK_OBJECT_TYPE_DEVICE_MEMORY, Name, Index); }
    void SetName(VkDevice Device, VkFramebuffer Framebuffer, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(Framebuffer), VK_OBJECT_TYPE_FRAMEBUFFER, Name, Index); }
    void SetName(VkDevice Device, VkImage Image, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(Image), VK_OBJECT_TYPE_IMAGE, Name, Index); }
    void SetName(VkDevice Device, VkImageView ImageView, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(ImageView), VK_OBJECT_TYPE_IMAGE_VIEW, Name, Index); }
    void SetName(VkDevice Device, VkPipeline Pipeline, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(Pipeline), VK_OBJECT_TYPE_PIPELINE, Name, Index); }
    void SetName(VkDevice Device, VkPipelineLayout PipelineLayout, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(PipelineLayout), VK_OBJECT_TYPE_PIPELINE_LAYOUT, Name, Index); }
    void SetName(VkDevice Device, VkQueryPool QueryPool, const std::string& Name, uint32_t Index) { SetName( Device, uint64_t(QueryPool), VK_OBJECT_TYPE_QUERY_POOL, Name, Index); }
    void SetName(VkDevice Device, VkQueue Queue, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(Queue), VK_OBJECT_TYPE_QUEUE, Name, Index); }
    void SetName(VkDevice Device, VkRenderPass RenderPass, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(RenderPass), VK_OBJECT_TYPE_RENDER_PASS, Name, Index); }
    void SetName(VkDevice Device, VkSampler Sampler, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(Sampler), VK_OBJECT_TYPE_SAMPLER, Name, Index); }
    void SetName(VkDevice Device, VkSemaphore Semaphore, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(Semaphore), VK_OBJECT_TYPE_SEMAPHORE, Name, Index); }
    void SetName(VkDevice Device, VkShaderModule ShaderModule, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(ShaderModule), VK_OBJECT_TYPE_SHADER_MODULE, Name, Index); }
    void SetName(VkDevice Device, VkSwapchainKHR SwapchainKHR, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(SwapchainKHR), VK_OBJECT_TYPE_SWAPCHAIN_KHR, Name, Index); }
    void SetName(VkDevice Device, VkAccelerationStructureKHR AccelerationStructure, const std::string& Name, uint32_t Index) { SetName(Device, uint64_t(AccelerationStructure), VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR, Name, Index); }
}