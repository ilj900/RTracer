#pragma once

#include "entity.h"

#include <queue>
#include <vector>

namespace ECS
{
    class FEntityManager
    {
    public:
        /// In constructor we just fill in the array of available entities
        FEntityManager();
        /// Fetch a new entity from array of available entities
        FEntity CreateEntity();
        /// Push destroyed entity into array of available entities
        void DestroyEntity(FEntity Entity);
        /// Set entity's signature, to mark it has according components
        void SetSignature(FEntity Entity, FSignature Signature);
        /// With signature you can check whether entity has component or not
        FSignature GetSignature(FEntity Entity);

    private:
        std::queue<FEntity> AvailableEntities{};
        std::vector<FSignature> Signatures{};
        uint32_t LivingEntitiesCount{};
    };

}