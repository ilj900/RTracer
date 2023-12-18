#include "vk_functions.h"
#include "vk_debug.h"

#include <cassert>

namespace V
{
    void SetName(VkDevice Device, uint64_t Object, VkObjectType ObjectType, const std::string& Name) {
#ifndef NDEBUG
        VkDebugUtilsObjectNameInfoEXT DebugUtilsObjectNameInfo{};
        DebugUtilsObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        DebugUtilsObjectNameInfo.objectType = ObjectType;
        DebugUtilsObjectNameInfo.objectHandle = Object;
        DebugUtilsObjectNameInfo.pObjectName = Name.c_str();

        VkResult Result = vkSetDebugUtilsObjectNameEXT(Device, &DebugUtilsObjectNameInfo);
        assert((Result == VK_SUCCESS) && "Failed to give a name to a vulkan object!");
#endif
    }

    void SetName(VkDevice Device, VkBuffer Buffer, const std::string& Name) { SetName(Device, uint64_t(Buffer), VK_OBJECT_TYPE_BUFFER, Name); }
    void SetName(VkDevice Device, VkBufferView BufferView, const std::string& Name) { SetName(Device, uint64_t(BufferView), VK_OBJECT_TYPE_BUFFER_VIEW, Name); }
    void SetName(VkDevice Device, VkCommandBuffer CommandBuffer, const std::string& Name) { SetName(Device, uint64_t(CommandBuffer), VK_OBJECT_TYPE_COMMAND_BUFFER, Name); }
    void SetName(VkDevice Device, VkCommandPool CommandPool, const std::string& Name) { SetName(Device, uint64_t(CommandPool), VK_OBJECT_TYPE_COMMAND_POOL, Name); }
    void SetName(VkDevice Device, VkDescriptorPool DescriptorPool, const std::string& Name) { SetName(Device, uint64_t(DescriptorPool), VK_OBJECT_TYPE_DESCRIPTOR_POOL, Name); }
    void SetName(VkDevice Device, VkDescriptorSet DescriptorSet, const std::string& Name) { SetName(Device, uint64_t(DescriptorSet), VK_OBJECT_TYPE_DESCRIPTOR_SET, Name); }
    void SetName(VkDevice Device, VkDescriptorSetLayout DescriptorSetLayout, const std::string& Name) { SetName(Device, uint64_t(DescriptorSetLayout), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, Name); }
    void SetName(VkDevice Device, VkDeviceMemory DeviceMemory, const std::string& Name) { SetName(Device, uint64_t(DeviceMemory), VK_OBJECT_TYPE_DEVICE_MEMORY, Name); }
    void SetName(VkDevice Device, VkFramebuffer Framebuffer, const std::string& Name) { SetName(Device, uint64_t(Framebuffer), VK_OBJECT_TYPE_FRAMEBUFFER, Name); }
    void SetName(VkDevice Device, VkImage Image, const std::string& Name) { SetName(Device, uint64_t(Image), VK_OBJECT_TYPE_IMAGE, Name); }
    void SetName(VkDevice Device, VkImageView ImageView, const std::string& Name) { SetName(Device, uint64_t(ImageView), VK_OBJECT_TYPE_IMAGE_VIEW, Name); }
    void SetName(VkDevice Device, VkPipeline Pipeline, const std::string& Name) { SetName(Device, uint64_t(Pipeline), VK_OBJECT_TYPE_PIPELINE, Name); }
    void SetName(VkDevice Device, VkPipelineLayout PipelineLayout, const std::string& Name) { SetName(Device, uint64_t(PipelineLayout), VK_OBJECT_TYPE_PIPELINE_LAYOUT, Name); }
    void SetName(VkDevice Device, VkQueryPool QueryPool, const std::string& Name) { SetName( Device, uint64_t(QueryPool), VK_OBJECT_TYPE_QUERY_POOL, Name); }
    void SetName(VkDevice Device, VkQueue Queue, const std::string& Name) { SetName(Device, uint64_t(Queue), VK_OBJECT_TYPE_QUEUE, Name); }
    void SetName(VkDevice Device, VkRenderPass RenderPass, const std::string& Name) { SetName(Device, uint64_t(RenderPass), VK_OBJECT_TYPE_RENDER_PASS, Name); }
    void SetName(VkDevice Device, VkSampler Sampler, const std::string& Name) { SetName(Device, uint64_t(Sampler), VK_OBJECT_TYPE_SAMPLER, Name); }
    void SetName(VkDevice Device, VkSemaphore Semaphore, const std::string& Name) { SetName(Device, uint64_t(Semaphore), VK_OBJECT_TYPE_SEMAPHORE, Name); }
    void SetName(VkDevice Device, VkShaderModule ShaderModule, const std::string& Name) { SetName(Device, uint64_t(ShaderModule), VK_OBJECT_TYPE_SHADER_MODULE, Name); }
    void SetName(VkDevice Device, VkSwapchainKHR SwapchainKHR, const std::string& Name) { SetName(Device, uint64_t(SwapchainKHR), VK_OBJECT_TYPE_SWAPCHAIN_KHR, Name); }
    void SetName(VkDevice Device, VkAccelerationStructureKHR AccelerationStructure, const std::string& Name) { SetName(Device, uint64_t(AccelerationStructure), VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR, Name); }
}