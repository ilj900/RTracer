#include "vk_functions.h"
#include "vk_debug.h"

namespace V
{
    void SetName(VkDevice Device, uint64_t Object, VkDebugReportObjectTypeEXT ObjectType, const std::string& Name) {
        VkDebugMarkerObjectNameInfoEXT DebugMarkerObjectNameInfo{};
        DebugMarkerObjectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        DebugMarkerObjectNameInfo.objectType = ObjectType;
        DebugMarkerObjectNameInfo.object = Object;
        DebugMarkerObjectNameInfo.pObjectName = Name.c_str();
        vkDebugMarkerSetObjectNameEXT(Device, &DebugMarkerObjectNameInfo);
    }

    void SetName(VkDevice Device, VkBuffer Buffer, const std::string& Name) { SetName(Device, uint64_t(Buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, Name); }
    void SetName(VkDevice Device, VkBufferView BufferView, const std::string& Name) { SetName(Device, uint64_t(BufferView), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, Name); }
    void SetName(VkDevice Device, VkCommandBuffer CommandBuffer, const std::string& Name) { SetName(Device, uint64_t(CommandBuffer), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, Name); }
    void SetName(VkDevice Device, VkCommandPool CommandPool, const std::string& Name) { SetName(Device, uint64_t(CommandPool), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT, Name); }
    void SetName(VkDevice Device, VkDescriptorPool DescriptorPool, const std::string& Name) { SetName(Device, uint64_t(DescriptorPool), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, Name); }
    void SetName(VkDevice Device, VkDescriptorSet DescriptorSet, const std::string& Name) { SetName(Device, uint64_t(DescriptorSet), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, Name); }
    void SetName(VkDevice Device, VkDescriptorSetLayout DescriptorSetLayout, const std::string& Name) { SetName(Device, uint64_t(DescriptorSetLayout), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, Name); }
    void SetName(VkDevice Device, VkDeviceMemory DeviceMemory, const std::string& Name) { SetName(Device, uint64_t(DeviceMemory), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, Name); }
    void SetName(VkDevice Device, VkFramebuffer Framebuffer, const std::string& Name) { SetName(Device, uint64_t(Framebuffer), VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, Name); }
    void SetName(VkDevice Device, VkImage Image, const std::string& Name) { SetName(Device, uint64_t(Image), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, Name); }
    void SetName(VkDevice Device, VkImageView ImageView, const std::string& Name) { SetName(Device, uint64_t(ImageView), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, Name); }
    void SetName(VkDevice Device, VkPipeline Pipeline, const std::string& Name) { SetName(Device, uint64_t(Pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, Name); }
    void SetName(VkDevice Device, VkPipelineLayout PipelineLayout, const std::string& Name) { SetName(Device, uint64_t(PipelineLayout), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, Name); }
    void SetName(VkDevice Device, VkQueryPool QueryPool, const std::string& Name) { SetName( Device, uint64_t(QueryPool), VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, Name); }
    void SetName(VkDevice Device, VkQueue Queue, const std::string& Name) { SetName(Device, uint64_t(Queue), VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, Name); }
    void SetName(VkDevice Device, VkRenderPass RenderPass, const std::string& Name) { SetName(Device, uint64_t(RenderPass), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, Name); }
    void SetName(VkDevice Device, VkSampler Sampler, const std::string& Name) { SetName(Device, uint64_t(Sampler), VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, Name); }
    void SetName(VkDevice Device, VkSemaphore Semaphore, const std::string& Name) { SetName(Device, uint64_t(Semaphore), VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, Name); }
    void SetName(VkDevice Device, VkShaderModule ShaderModule, const std::string& Name) { SetName(Device, uint64_t(ShaderModule), VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, Name); }
    void SetName(VkDevice Device, VkSwapchainKHR SwapchainKHR, const std::string& Name) { SetName(Device, uint64_t(SwapchainKHR), VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, Name); }
}