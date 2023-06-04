#include "component_manager.h"

namespace ECS
{
    static FComponentManager ComponentManager{};

    FComponentManager* GetComponentManager()
    {
        return &ComponentManager;
    }
}