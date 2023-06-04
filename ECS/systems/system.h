#pragma once

#include "entities/entity.h"

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
            
        public:
            std::set<FEntity> Entities;
        };
    }
}