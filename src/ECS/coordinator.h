#pragma once

#include "entity_manager.h"
#include "component_manager.h"
#include "system_manager.h"

namespace ECS
{
    /// Coordinator is NOT a singleton.
    class FCoordinator
    {
    public:

        FCoordinator() = default;
        FCoordinator operator=(const FCoordinator* Other) = delete;
        FCoordinator(const FCoordinator& Other) = delete;

        void Init()
        {
            ComponentManager = ECS::GetComponentManager();
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

            auto Signature = EntityManager->GetSignature(Entity);
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
		T& GetComponentByIndex(uint32_t ComponentIndex)
		{
			return ComponentManager->GetComponentByIndex<T>(ComponentIndex);
		}

        template<typename T>
        uint32_t GetOffset(FEntity Entity)
        {
            return ComponentManager->GetOffset<T>(Entity);
        }

        template<typename T>
        size_t GetIndex(FEntity Entity)
        {
            return ComponentManager->GetIndex<T>(Entity);
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

        /// Returns a pointer to the ComponentArray's data. Perhaps we should return const pointer.
        /// TODO: Check whether const can be applied
        template<typename T>
        T* Data()
        {
            return ComponentManager->Data<T>();
        }

        template<typename T>
        void* Data(FEntity Entity)
        {
            return ComponentManager->Data<T>(Entity);
        }

        /// Returns a size of a ComponentArray's data
        template<typename T>
        size_t Size()
        {
            return ComponentManager->Size<T>();
        }

    private:
        FComponentManager* ComponentManager;
        std::unique_ptr<FEntityManager> EntityManager;
        std::unique_ptr<FSystemManager> SystemManager;
    };

    /// Coordinator can be accessed from all around the code
    FCoordinator& GetCoordinator();
}

#define COORDINATOR() ECS::GetCoordinator()
