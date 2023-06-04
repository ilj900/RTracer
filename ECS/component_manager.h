#pragma once

#include "component_array.h"

#include <memory>
#include <unordered_map>

namespace ECS
{
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

        /// It's here just to call ComponentArray's method and being called by Coordinator method.
        /// Making all private fields public could solve this problem. If it's a problem
        template<typename T>
        T* Data()
        {
            return GetComponentArray<T>()->Data();
        }

        /// It's here just to call ComponentArray's method and being called by Coordinator method.
        /// Making all private fields public could solve this problem. If it's a problem
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

    FComponentManager* GetComponentManager();
}