#pragma once

#include "entities/entity.h"
#include "systems/system.h"

#include <array>
#include <cassert>
#include <memory>
#include <queue>
#include <set>
#include <unordered_map>

namespace ECS
{
    class FEntityManager
    {
    public:
        FEntityManager()
        {
            for (FEntity Entity = 0; Entity < MAX_ENTITIES; ++Entity)
            {
                AvailableEntities.push(Entity);
            }
        }

        FEntity CreateEntity()
        {
            assert(LivingEntitiesCount < MAX_ENTITIES && "To many entities, can't create more!");

            FEntity ID = AvailableEntities.front();
            AvailableEntities.pop();
            ++LivingEntitiesCount;

            return ID;
        }

        void DestroyEntity(FEntity Entity)
        {
            assert(Entity < MAX_ENTITIES && "Entity out of range!");

            Signatures[Entity].reset();
            AvailableEntities.push(Entity);
            --LivingEntitiesCount;
        }

        void SetSignature(FEntity Entity, FSignature Signature)
        {
            assert(Entity < MAX_ENTITIES && "Entity out of range!");

            Signatures[Entity] = Signature;
        }

        FSignature GetSignature(FEntity Entity)
        {
            assert(Entity < MAX_ENTITIES && "Entity out of range!");

            return Signatures[Entity];
        }

    private:
        std::queue<FEntity> AvailableEntities{};
        std::array<FSignature, MAX_ENTITIES> Signatures{};
        uint32_t LivingEntitiesCount{};
    };

    class IComponentArray
    {
    public:
        virtual ~IComponentArray() = default;
        virtual void EntityDestroyed(FEntity Entity) = 0;
    };

    template<typename T>
    class TComponentArray : public IComponentArray
    {
    public:
        void InsertData(FEntity Entity, T Component)
        {
            assert(EntityToIndexMap.find(Entity) == EntityToIndexMap.end() && "Entity already has that component!");

            size_t NewIndex = ArraySize;
            EntityToIndexMap[Entity] = NewIndex;
            IndexToEntityMap[NewIndex] = Entity;
            ComponentArray[NewIndex] = Component;
            ++ArraySize;
        }

        void RemoveData(FEntity Entity)
        {
            assert(EntityToIndexMap.find(Entity) != EntityToIndexMap.end() && "removing non-existing component!");

            size_t IndexOfRemovedEntity = EntityToIndexMap[Entity];
            size_t IndexOfLastElement = ArraySize - 1;
            ComponentArray[IndexOfRemovedEntity] = ComponentArray[IndexOfLastElement];

            FEntity EntityOfLastElement = IndexToEntityMap[IndexOfLastElement];
            EntityToIndexMap[EntityOfLastElement] = IndexOfRemovedEntity;
            IndexToEntityMap[IndexOfRemovedEntity] = EntityOfLastElement;

            EntityToIndexMap.erase(Entity);
            IndexToEntityMap.erase(IndexOfLastElement);

            --ArraySize;
        }

        T& GetData(FEntity Entity)
        {
            assert(EntityToIndexMap.find(Entity) != EntityToIndexMap.end() && "Entity doesn't have such component!");

            return ComponentArray[EntityToIndexMap[Entity]];
        }

        T* Data()
        {
            return ComponentArray.data();
        }

        size_t Size()
        {
            return ArraySize * sizeof(T);
        }

        void EntityDestroyed(FEntity Entity) override
        {
            if (EntityToIndexMap.find(Entity) != EntityToIndexMap.end())
            {
                RemoveData(Entity);
            }
        }
    private:
        std::array<T, MAX_ENTITIES> ComponentArray;
        std::unordered_map<FEntity, size_t> EntityToIndexMap;
        std::unordered_map<size_t, FEntity> IndexToEntityMap;
        size_t ArraySize;
    };

    class FComponentManager
    {
    public:
        template<typename T>
        void RegisterComponent()
        {
            const char* TypeName = typeid(T).name();

            assert(ComponentTypes.find(TypeName) == ComponentTypes.end() && "Registering component types more that once!");

            ComponentTypes.insert({TypeName, NextComponentType});
            ComponentArrays.insert({TypeName, std::make_shared<TComponentArray<T>>()});

            ++NextComponentType;
        }

        template<typename T>
        FComponentType GetComponentType()
        {
            const char* TypeName = typeid(T).name();

            assert(ComponentTypes.find(TypeName) != ComponentTypes.end() && "Component type not registered!");

            return ComponentTypes[TypeName];
        }

        template<typename T>
        void AddComponent(FEntity Entity, T Component)
        {
            GetComponentArray<T>()->InsertData(Entity, Component);
        }

        template<typename T>
        void RemoveComponent(FEntity Entity)
        {
            GetComponentArray<T>()->RemoveData(Entity);
        }

        template<typename T>
        T& GetComponent(FEntity Entity)
        {
            return GetComponentArray<T>()->GetData(Entity);
        }

        template<typename T>
        T* Data()
        {
            return GetComponentArray<T>()->Data();
        }

        template<typename T>
        size_t Size()
        {
            return GetComponentArray<T>()->Size();
        }

        void EntityDestroyed(FEntity Entity)
        {
            for (auto const& Pair : ComponentArrays)
            {
                auto const& Component = Pair.second;

                Component->EntityDestroyed(Entity);
            }
        }
    private:
        std::unordered_map<const char*, FComponentType> ComponentTypes{};
        std::unordered_map<const char*, std::shared_ptr<IComponentArray>> ComponentArrays{};
        FComponentType NextComponentType{};

        template<typename T>
        std::shared_ptr<TComponentArray<T>> GetComponentArray()
        {
            const char* TypeName = typeid(T).name();

            assert(ComponentTypes.find(TypeName) != ComponentTypes.end() && "Component not registered before use!");

            return std::static_pointer_cast<TComponentArray<T>>(ComponentArrays[TypeName]);
        }
    };

    class FSystemManager
    {
    public:
        template<typename T>
        std::shared_ptr<T> RegisterSystem()
        {
            const char* TypeName = typeid(T).name();

            assert(Systems.find(TypeName) == Systems.end() && "Registering system more than once!");

            auto System = std::make_shared<T>();
            Systems.insert({TypeName, System});
            return System;
        }

        template<typename T>
        std::shared_ptr<T> GetSystem()
        {
            const char* TypeName = typeid(T).name();
            assert(Systems.find(TypeName) != Systems.end() && "System not registered!");

            return std::static_pointer_cast<T>(Systems.find(TypeName)->second);
        }

        template<typename T>
        void SetSignature(FSignature Signature)
        {
            const char* TypeName = typeid(T).name();

            assert(Systems.find(TypeName) != Systems.end() && "System doesn't exist!");

            Signatures.insert({TypeName, Signature});
        }

        void EntityDestroyed(FEntity Entity)
        {
            for (auto const& Pair : Systems)
            {
                auto const& System = Pair.second;
                System->Entities.erase(Entity);
            }
        }

        void EntitySignatureChanged(FEntity Entity, FSignature EntitySignature)
        {
            for(auto const& Pair : Systems)
            {
                auto const& Type = Pair.first;
                auto const& System = Pair.second;
                auto const& SystemSignature = Signatures[Type];

                if ((EntitySignature & SystemSignature) == SystemSignature)
                {
                    System->Entities.insert(Entity);
                }
                else
                {
                    System->Entities.erase(Entity);
                }
            }
        }
    private:
        std::unordered_map<const char*, FSignature> Signatures{};
        std::unordered_map<const char*, std::shared_ptr<FSystem>> Systems{};
    };

    class FCoordinator
    {
    public:

        FCoordinator() = default;
        FCoordinator operator=(const FCoordinator* Other) = delete;
        FCoordinator(const FCoordinator& Other) = delete;

        void Init()
        {
            ComponentManager = std::make_unique<FComponentManager>();
            EntityManager = std::make_unique<FEntityManager>();
            SystemManager = std::make_unique<FSystemManager>();
        }

        FEntity CreateEntity()
        {
            return EntityManager->CreateEntity();
        }

        void DestroyEntity(FEntity Entity)
        {
            EntityManager->DestroyEntity(Entity);
            ComponentManager->EntityDestroyed(Entity);
        }

        template<typename T>
        void RegisterComponent()
        {
            ComponentManager->template RegisterComponent<T>();
        }

        template<typename T>
        void AddComponent(FEntity Entity, T Component)
        {
            ComponentManager->AddComponent<T>(Entity, Component);

            auto Signature = EntityManager->GetSignature(Entity);
            Signature.set(ComponentManager->template GetComponentType<T>(), true);
            EntityManager->SetSignature(Entity, Signature);

            SystemManager->EntitySignatureChanged(Entity, Signature);
        }

        template<typename T>
        void RemoveComponent(FEntity Entity)
        {
            ComponentManager->RemoveComponent<T>(Entity);

            auto Signature = mEntityManager->GetSignature(Entity);
            Signature.set(ComponentManager->GetComponentType<T>(), false);
            EntityManager->SetSignature(Entity, Signature);

            SystemManager->EntitySignatureChanged(Entity, Signature);
        }

        template<typename T>
        T& GetComponent(FEntity Entity)
        {
            return ComponentManager->GetComponent<T>(Entity);
        }

        template<typename T>
        FComponentType GetComponentType()
        {
            return ComponentManager->GetComponentType<T>();
        }

        template<typename T>
        std::shared_ptr<T> RegisterSystem()
        {
            return SystemManager->template RegisterSystem<T>();
        }

        template<typename T>
        void SetSystemSignature(FSignature Signature)
        {
            SystemManager->template SetSignature<T>(Signature);
        }

        template<typename T>
        std::shared_ptr<T> GetSystem()
        {
            return SystemManager->template GetSystem<T>();
        }

        template<typename T>
        T* Data()
        {
            return ComponentManager->Data<T>();
        }

        template<typename T>
        size_t Size()
        {
            return ComponentManager->Size<T>();
        }

    private:
        std::unique_ptr<FComponentManager> ComponentManager;
        std::unique_ptr<FEntityManager> EntityManager;
        std::unique_ptr<FSystemManager> SystemManager;
    };

    FCoordinator& GetCoordinator();
}