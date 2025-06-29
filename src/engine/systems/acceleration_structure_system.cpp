#pragma once

#include "mesh_component.h"
#include "device_mesh_component.h"
#include "device_transform_component.h"
#include "transform_component.h"
#include "acceleration_structure_component.h"
#include "device_renderable_component.h"
#include "transform_system.h"
#include "renderable_system.h"
#include "mesh_system.h"

#include "acceleration_structure_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FAccelerationStructureSystem::Init(uint32_t NumberOfSimultaneousSubmits)
        {
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(COMPONENTS::FMeshInstanceComponent) * MAX_INSTANCE_COUNT,
                                       VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Acceleration_Structure");
        }

        void FAccelerationStructureSystem::Terminate()
        {
            VK_CONTEXT()->DestroyAccelerationStructure(TLAS);
        }

        bool FAccelerationStructureSystem::Update()
        {
			bool bAnyUpdate = false;

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FMeshInstanceComponent>();
				UpdateTLAS();
			}

			return bAnyUpdate;
        }

        bool FAccelerationStructureSystem::Update(int Index)
        {
			bool bAnyUpdate = false;

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FMeshInstanceComponent>(Index);
				UpdateTLAS();
			}

			return bAnyUpdate;
        }

        FEntity FAccelerationStructureSystem::CreateInstance(FEntity Entity, const FVector3& Position, const FVector3& Direction, const FVector3& Up, const FVector3& Scale)
        {
            FEntity NewMeshInstance = COORDINATOR().CreateEntity();

            COORDINATOR().AddComponent<ECS::COMPONENTS::FTransformComponent>(NewMeshInstance, {Position, Direction, Up, Scale});
            COORDINATOR().AddComponent<ECS::COMPONENTS::FDeviceTransformComponent>(NewMeshInstance, {});
            TRANSFORM_SYSTEM()->SyncTransform(NewMeshInstance);

            auto& ModelMatrix = COORDINATOR().GetComponent<ECS::COMPONENTS::FDeviceTransformComponent>(NewMeshInstance).ModelMatrix.Data;
            auto& BLAS = GetComponent<ECS::COMPONENTS::FAccelerationStructureComponent>(Entity);

			COORDINATOR().AddComponent<ECS::COMPONENTS::FMeshInstanceComponent>(NewMeshInstance, {});
            auto& MeshInstanceComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FMeshInstanceComponent>(NewMeshInstance);
            MeshInstanceComponent.transform = { ModelMatrix[0].X, ModelMatrix[0].Y, ModelMatrix[0].Z, ModelMatrix[0].W,
                                                ModelMatrix[1].X, ModelMatrix[1].Y, ModelMatrix[1].Z, ModelMatrix[1].W,
                                                ModelMatrix[2].X, ModelMatrix[2].Y, ModelMatrix[2].Z, ModelMatrix[2].W};
            MeshInstanceComponent.instanceCustomIndex = COORDINATOR().GetIndex<ECS::COMPONENTS::FMeshInstanceComponent>(NewMeshInstance);
            MeshInstanceComponent.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
            MeshInstanceComponent.mask = 0xFF;
            MeshInstanceComponent.accelerationStructureReference = VK_CONTEXT()->GetASDeviceAddressInfo(BLAS.AccelerationStructure);
            MeshInstanceComponent.instanceShaderBindingTableRecordOffset = 0;

			COORDINATOR().AddComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(NewMeshInstance, {});
			auto& DeviceRenderableComponent = RENDERABLE_SYSTEM()->GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(NewMeshInstance);
			DeviceRenderableComponent.MeshIndex = Entity;
			DeviceRenderableComponent.RenderableIndex = MeshInstanceComponent.instanceCustomIndex;

            MarkDirty(NewMeshInstance);
            bIsDirty = true;
            InstanceCount++;
            return NewMeshInstance;
        }

		void FAccelerationStructureSystem::UpdateInstancePosition(FEntity Entity)
		{
			auto& MeshInstanceComponent = GetComponent<COMPONENTS::FMeshInstanceComponent>(Entity);
			auto& ModelMatrix = COORDINATOR().GetComponent<ECS::COMPONENTS::FDeviceTransformComponent>(Entity).ModelMatrix.Data;
			MeshInstanceComponent.transform = { ModelMatrix[0].X, ModelMatrix[0].Y, ModelMatrix[0].Z, ModelMatrix[0].W,
												ModelMatrix[1].X, ModelMatrix[1].Y, ModelMatrix[1].Z, ModelMatrix[1].W,
												ModelMatrix[2].X, ModelMatrix[2].Y, ModelMatrix[2].Z, ModelMatrix[2].W};
			MarkDirty(Entity);
			bIsDirty = true;
		}

        void FAccelerationStructureSystem::UpdateTLAS()
        {
            if (bIsDirty)
            {
				if (TLAS.AccelerationStructure == VK_NULL_HANDLE)
				{
					TLAS = VK_CONTEXT()->GenerateTlas(DeviceBuffer, InstanceCount);
				}
                bIsDirty = false;
            }
        }
    }
}