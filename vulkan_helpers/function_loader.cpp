#include "function_loader.h"

#include "vulkan_wrappers.h"

void FVulkanFunctionLoader::LoadFunctions(VkInstance &Instance)
{
    vkCreateDebugUtilsMessengerEXT = V::LoadInstanceFunction<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT", Instance);
    vkDestroyDebugUtilsMessengerEXT = V::LoadInstanceFunction<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT", Instance);
    vkSetDebugUtilsObjectNameEXT = V::LoadInstanceFunction<PFN_vkSetDebugUtilsObjectNameEXT>("vkSetDebugUtilsObjectNameEXT", Instance);
    vkCreateAccelerationStructureKHR = V::LoadInstanceFunction<PFN_vkCreateAccelerationStructureKHR>("vkCreateAccelerationStructureKHR", Instance);
    vkDestroyAccelerationStructureKHR = V::LoadInstanceFunction<PFN_vkDestroyAccelerationStructureKHR>("vkDestroyAccelerationStructureKHR", Instance);
    vkGetAccelerationStructureBuildSizesKHR = V::LoadInstanceFunction<PFN_vkGetAccelerationStructureBuildSizesKHR>("vkGetAccelerationStructureBuildSizesKHR", Instance);
    vkCmdBuildAccelerationStructuresKHR = V::LoadInstanceFunction<PFN_vkCmdBuildAccelerationStructuresKHR>("vkCmdBuildAccelerationStructuresKHR", Instance);
    vkCmdWriteAccelerationStructuresPropertiesKHR = V::LoadInstanceFunction<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>("vkCmdWriteAccelerationStructuresPropertiesKHR", Instance);
    vkCmdCopyAccelerationStructureKHR = V::LoadInstanceFunction<PFN_vkCmdCopyAccelerationStructureKHR>("vkCmdCopyAccelerationStructureKHR", Instance);
    vkGetAccelerationStructureDeviceAddressKHR = V::LoadInstanceFunction<PFN_vkGetAccelerationStructureDeviceAddressKHR>("vkGetAccelerationStructureDeviceAddressKHR", Instance);
    vkGetRayTracingShaderGroupHandlesKHR = V::LoadInstanceFunction<PFN_vkGetRayTracingShaderGroupHandlesKHR>("vkGetRayTracingShaderGroupHandlesKHR", Instance);
    vkCreateRayTracingPipelinesKHR = V::LoadInstanceFunction<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR", Instance);
    vkCmdTraceRaysKHR = V::LoadInstanceFunction<PFN_vkCmdTraceRaysKHR>("vkCmdTraceRaysKHR", Instance);
}