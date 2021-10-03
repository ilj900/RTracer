#pragma once

#include "entity_manager.h"
#include "component_manager.h"
#include "system_manager.h"

namespace ECS
{
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