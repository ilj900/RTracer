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
                T& Component = GetComponentManager()->GetComponent<T>(Entity);
                return Component;
            };

			template<typename T>
			T& GetComponentByIndex(uint32_t ComponentIndex)
			{
				T& Component = GetComponentManager()->GetComponentByIndex<T>(ComponentIndex);
				return Component;
			};

        public:
            std::set<FEntity> Entities;
        };
    }
}