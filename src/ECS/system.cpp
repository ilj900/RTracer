#include "system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FSystem::RegisterEntity(FEntity Entity)
        {
            Entities.insert(Entity);
        }

        void FSystem::UnregisterEntity(FEntity Entity)
        {
            Entities.erase(Entity);
        }

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
