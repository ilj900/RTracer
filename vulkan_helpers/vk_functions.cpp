#include "vk_functions.h"

#include <stdexcept>

auto CheckNotNullptr = [](void* FunctionPointer, const std::string& FunctionName)
{
    if (FunctionPointer == nullptr)
    {
        throw std::runtime_error("Failed to load function: " + FunctionName + "\n");
    }
};

/// Load an instance function
template <class T>
T LoadInstanceFunction(const std::string& FunctionName, VkInstance Instance)
{
    T Function = (T) vkGetInstanceProcAddr(Instance, FunctionName.c_str());
    CheckNotNullptr(Function, FunctionName);
    return Function;
}

/// Load a device function
template <class T>
T LoadDeviceFunction(const std::string& FunctionName, VkDevice Device)
{
    T Function = (T) vkGetDeviceProcAddr(Device, FunctionName.c_str());
    CheckNotNullptr(Function, FunctionName);
    return Function;
}

namespace V
{
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;
    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = nullptr;
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR = nullptr;
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = nullptr;
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = nullptr;
    PFN_vkCmdWriteAccelerationStructuresPropertiesKHR vkCmdWriteAccelerationStructuresPropertiesKHR = nullptr;
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = nullptr;
    PFN_vkCmdCopyAccelerationStructureKHR vkCmdCopyAccelerationStructureKHR = nullptr;
    PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = nullptr;
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = nullptr;
    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = nullptr;
    PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT = nullptr;

    void LoadVkFunctions(VkInstance Instance)
    {
        vkCreateDebugUtilsMessengerEXT = LoadInstanceFunction<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT", Instance);
        vkDestroyDebugUtilsMessengerEXT = LoadInstanceFunction<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT", Instance);
        vkSetDebugUtilsObjectNameEXT = LoadInstanceFunction<PFN_vkSetDebugUtilsObjectNameEXT>("vkSetDebugUtilsObjectNameEXT", Instance);
        vkCreateAccelerationStructureKHR = LoadInstanceFunction<PFN_vkCreateAccelerationStructureKHR>("vkCreateAccelerationStructureKHR", Instance);
        vkDestroyAccelerationStructureKHR = LoadInstanceFunction<PFN_vkDestroyAccelerationStructureKHR>("vkDestroyAccelerationStructureKHR", Instance);
        vkGetAccelerationStructureBuildSizesKHR = LoadInstanceFunction<PFN_vkGetAccelerationStructureBuildSizesKHR>("vkGetAccelerationStructureBuildSizesKHR", Instance);
        vkCmdBuildAccelerationStructuresKHR = LoadInstanceFunction<PFN_vkCmdBuildAccelerationStructuresKHR>("vkCmdBuildAccelerationStructuresKHR", Instance);
        vkCmdWriteAccelerationStructuresPropertiesKHR = LoadInstanceFunction<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>("vkCmdWriteAccelerationStructuresPropertiesKHR", Instance);
        vkCmdCopyAccelerationStructureKHR = LoadInstanceFunction<PFN_vkCmdCopyAccelerationStructureKHR>("vkCmdCopyAccelerationStructureKHR", Instance);
        vkGetAccelerationStructureDeviceAddressKHR = LoadInstanceFunction<PFN_vkGetAccelerationStructureDeviceAddressKHR>("vkGetAccelerationStructureDeviceAddressKHR", Instance);
        vkGetRayTracingShaderGroupHandlesKHR = LoadInstanceFunction<PFN_vkGetRayTracingShaderGroupHandlesKHR>("vkGetRayTracingShaderGroupHandlesKHR", Instance);
        vkCreateRayTracingPipelinesKHR = LoadInstanceFunction<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR", Instance);
        vkCmdTraceRaysKHR = LoadInstanceFunction<PFN_vkCmdTraceRaysKHR>("vkCmdTraceRaysKHR", Instance);
        vkDebugMarkerSetObjectNameEXT = LoadInstanceFunction<PFN_vkDebugMarkerSetObjectNameEXT>("vkDebugMarkerSetObjectNameEXT", Instance);
    }
}