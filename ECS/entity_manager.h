#pragma once

#include "entities/entity.h"

#include <array>
#include <queue>

namespace ECS
{
    class FEntityManager
    {
    public:
        FEntityManager();

        FEntity CreateEntity();

        void DestroyEntity(FEntity Entity);

        void SetSignature(FEntity Entity, FSignature Signature);

        FSignature GetSignature(FEntity Entity);

    private:
        std::queue<FEntity> AvailableEntities{};
        std::array<FSignature, MAX_ENTITIES> Signatures{};
        uint32_t LivingEntitiesCount{};
    };

}