#include "entity_manager.h"

#include <cassert>

namespace ECS
{
    FEntityManager::FEntityManager()
    {
        for (FEntity Entity = 0; Entity < MAX_ENTITIES - 1; ++Entity)
        {
            AvailableEntities.push(Entity);
        }

		Signatures.resize(MAX_ENTITIES - 1, 0);
    }

    FEntity FEntityManager::CreateEntity()
    {
        assert(LivingEntitiesCount < MAX_ENTITIES && "To many entities, can't create more!");

        FEntity ID = AvailableEntities.front();
        AvailableEntities.pop();
        ++LivingEntitiesCount;

        return ID;
    }

    void FEntityManager::DestroyEntity(FEntity Entity)
    {
        assert(Entity < MAX_ENTITIES && "Entity out of range!");

        Signatures[Entity].reset();
        AvailableEntities.push(Entity);
        --LivingEntitiesCount;
    }

    void FEntityManager::SetSignature(FEntity Entity, FSignature Signature)
    {
        assert(Entity < MAX_ENTITIES && "Entity out of range!");

        Signatures[Entity] = Signature;
    }

    FSignature FEntityManager::GetSignature(FEntity Entity)
    {
        assert(Entity < MAX_ENTITIES && "Entity out of range!");

        return Signatures[Entity];
    }
}