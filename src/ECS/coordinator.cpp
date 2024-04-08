#include "coordinator.h"

namespace ECS
{
    static FCoordinator Coordinator{};

    FCoordinator& GetCoordinator()
    {
        return Coordinator;
    }
}
