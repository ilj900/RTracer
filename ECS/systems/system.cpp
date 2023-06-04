#include "system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        std::set<FEntity>::iterator  FSystem::begin()
        {
            return Entities.begin();
        }

        std::set<FEntity>::iterator  FSystem::end()
        {
            return Entities.end();
        }
    }
}
