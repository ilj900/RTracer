#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <string>

namespace VKH {
    class FInstance
    {
    public:
        FInstance(const std::string& Name, uint32_t APIVersionMajor, uint32_t APIVersionMinor, uint32_t APIVersionPatch);
        ~FInstance();

        void Init();

        void InstancePushExtension(const char* ExtensionName);
        void InstancePushLayer(const char* LayerName);

    public:
        VkInstance Instance = nullptr;
        std::string Name = "";
        uint32_t Major = 1;
        uint32_t Minor = 0;
        uint32_t Patch = 0;

        std::vector<const char*> Extensions;
        std::vector<const char*> Layers;
    };
}