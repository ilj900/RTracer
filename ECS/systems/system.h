#pragma once

#include "entities/entity.h"

#include <set>

namespace ECS
{
    class FSystem
    {
    public:
        std::set<FEntity> Entities;
    };
}