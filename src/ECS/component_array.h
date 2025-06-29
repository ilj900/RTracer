#pragma once

#include "entity.h"

#include <array>
#include <cassert>
#include <unordered_map>

namespace ECS {
    class IComponentArray {
    public:
        virtual ~IComponentArray() = default;

        virtual void EntityDestroyed(FEntity Entity) = 0;
    };

    template<typename T>
    class TComponentArray : public IComponentArray {
    public:
        void InsertData(FEntity Entity, T Component) {
            assert(EntityToIndexMap.find(Entity) == EntityToIndexMap.end() && "Entity already has that component!");

            size_t NewIndex = ArraySize;
            EntityToIndexMap[Entity] = NewIndex;
            IndexToEntityMap[NewIndex] = Entity;
            ComponentArray[NewIndex] = Component;
            ++ArraySize;
        }

        void RemoveData(FEntity Entity) {
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

        /// Get data of a single entity
        T& GetData(FEntity Entity)
        {
            assert(EntityToIndexMap.find(Entity) != EntityToIndexMap.end() && "Entity doesn't have such component!");

            return ComponentArray[EntityToIndexMap[Entity]];
        }

		/// Get data of a single entity
		T& GetDataByIndex(uint32_t ComponentIndex)
		{
			assert(EntityToIndexMap.size() >= ComponentIndex && "Index is greater than the size of the component array!");

			return ComponentArray[ComponentIndex];
		}

        /// Get index of the stored entity's component.
        uint32_t GetIndex(FEntity Entity)
        {
            assert(EntityToIndexMap.find(Entity) != EntityToIndexMap.end() && "Entity doesn't have such component!");

            return EntityToIndexMap[Entity];
        }

        /// Get the offset for the stored entity's component.
        uint32_t GetOffset(FEntity Entity)
        {
            assert(EntityToIndexMap.find(Entity) != EntityToIndexMap.end() && "Entity doesn't have such component!");

            return EntityToIndexMap[Entity] * sizeof(T);
        }

        /// Get all data from the component array (Needed when we upload this data to GPU)
        T* Data() {
            return ComponentArray.data();
        }

        void* Data(FEntity Entity)
        {
            return ComponentArray.data() + EntityToIndexMap[Entity];
        }

        /// Get the total size of component's array
        size_t Size() {
            return ArraySize * sizeof(T);
        }

        void EntityDestroyed(FEntity Entity) override {
            if (EntityToIndexMap.find(Entity) != EntityToIndexMap.end()) {
                RemoveData(Entity);
            }
        }

    private:
        std::array <T, MAX_ENTITIES> ComponentArray;
        std::unordered_map <FEntity, size_t> EntityToIndexMap;
        std::unordered_map <size_t, FEntity> IndexToEntityMap;
        size_t ArraySize;
    };
}