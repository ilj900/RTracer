#pragma once

#include "entity.h"
#include "component_manager.h"

#include <cassert>
#include <set>

namespace ECS
{
    namespace SYSTEMS
    {
        class FSystem
        {
        public:
            FSystem() = default;
            std::set<FEntity>::iterator begin();
            std::set<FEntity>::iterator end();

            virtual void RegisterEntity(FEntity Entity);
            virtual void UnregisterEntity(FEntity Entity);

            template<typename T>
            T& GetComponent(FEntity Entity)
            {
                assert(Entities.find(Entity) != Entities.end() && "Entity doesn't have required component");
                T& Component = GetComponentManager()->GetComponent<T>(Entity);
                return Component;
            };

        public:
            std::set<FEntity> Entities;
        };
    }
}