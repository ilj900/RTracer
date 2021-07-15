#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <vector>


namespace VKH
{
    void InstancePushExtension(const char* ExtensionName);
    void InstancePushLayer(const char* LayerName);
    VkInstance CreateInstance(std::string& ApplicationName, uint32_t ApplicationVersion);
    void QuerryPhysicalDevices();

    static std::vector<const char*> Extensions;
    static std::vector<const char*> Layers;
}