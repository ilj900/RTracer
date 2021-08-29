#include "function_loader.h"

#include <stdexcept>

void FVulkanFunctionLoader::LoadFunctions(VkInstance &Instance)
{
    auto CheckNotNullptr = [](void* FunctionPointer, const std::string& FunctionName)
    {
        if (FunctionPointer == nullptr)
        {
            throw std::runtime_error("Failed to load function: " + FunctionName + "\n");
        }
    };
    vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
    CheckNotNullptr(vkCreateDebugUtilsMessengerEXT, "vkCreateDebugUtilsMessengerEXT");

    vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
    CheckNotNullptr(vkDestroyDebugUtilsMessengerEXT, "vkDestroyDebugUtilsMessengerEXT");

    vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetInstanceProcAddr(Instance, "vkSetDebugUtilsObjectNameEXT");
    CheckNotNullptr(vkSetDebugUtilsObjectNameEXT, "vkSetDebugUtilsObjectNameEXT");

    vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR) vkGetInstanceProcAddr(Instance, "vkCreateAccelerationStructureKHR");
    CheckNotNullptr(vkCreateAccelerationStructureKHR, "vkCreateAccelerationStructureKHR");

    vkDestroyAccelerationStructureKHR = (PFN_vkDestroyAccelerationStructureKHR) vkGetInstanceProcAddr(Instance, "vkDestroyAccelerationStructureKHR");
    CheckNotNullptr(vkDestroyAccelerationStructureKHR, "vkDestroyAccelerationStructureKHR");

    vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR) vkGetInstanceProcAddr(Instance, "vkGetAccelerationStructureBuildSizesKHR");
    CheckNotNullptr(vkGetAccelerationStructureBuildSizesKHR, "vkGetAccelerationStructureBuildSizesKHR");

    vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR) vkGetInstanceProcAddr(Instance, "vkCmdBuildAccelerationStructuresKHR");
    CheckNotNullptr(vkCmdBuildAccelerationStructuresKHR, "vkCmdBuildAccelerationStructuresKHR");

    vkCmdWriteAccelerationStructuresPropertiesKHR = (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)vkGetInstanceProcAddr(Instance, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    CheckNotNullptr(vkCmdWriteAccelerationStructuresPropertiesKHR, "vkCmdWriteAccelerationStructuresPropertiesKHR");

    vkCmdCopyAccelerationStructureKHR = (PFN_vkCmdCopyAccelerationStructureKHR)vkGetInstanceProcAddr(Instance, "vkCmdCopyAccelerationStructureKHR");
    CheckNotNullptr(vkCmdCopyAccelerationStructureKHR, "vkCmdCopyAccelerationStructureKHR");

    vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetInstanceProcAddr(Instance, "vkGetAccelerationStructureDeviceAddressKHR");
    CheckNotNullptr(vkGetAccelerationStructureDeviceAddressKHR, "vkGetAccelerationStructureDeviceAddressKHR");

    vkGetRayTracingShaderGroupHandlesKHR = (PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetInstanceProcAddr(Instance, "vkGetRayTracingShaderGroupHandlesKHR");
    CheckNotNullptr(vkGetRayTracingShaderGroupHandlesKHR, "vkGetRayTracingShaderGroupHandlesKHR");

    vkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)vkGetInstanceProcAddr(Instance, "vkCreateRayTracingPipelinesKHR");
    CheckNotNullptr(vkCreateRayTracingPipelinesKHR, "vkCreateRayTracingPipelinesKHR");

    vkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)vkGetInstanceProcAddr(Instance, "vkCmdTraceRaysKHR");
    CheckNotNullptr(vkCmdTraceRaysKHR, "vkCmdTraceRaysKHR");
}