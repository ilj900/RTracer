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
#include "vk_context.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FAccelerationStructureSystem::Init(int NumberOfSimultaneousSubmits)
        {
            BLASInstanceBuffer = GetResourceAllocator()->CreateBuffer(sizeof(VkAccelerationStructureInstanceKHR) * MAX_INSTANCE_COUNT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "V_BLAS_Instance_Buffer");

            for (uint32_t i = 0; i < MAX_INSTANCE_COUNT; ++i)
            {
                AvailableIndices.push(i);
            }
        }

        void FAccelerationStructureSystem::Update()
        {
            std::vector<VkDeviceSize> Sizes;
            std::vector<VkDeviceSize> Offsets;
            std::vector<void*> Data;

            auto& Coordinator = GetCoordinator();

            for (auto Entity : EntitiesToUpdate)
            {
                /// This check checks whether the data to be uploaded are in a continues block
                if (Offsets.size() > 0 && (Coordinator.GetOffset<ECS::COMPONENTS::FMeshInstanceComponent>(Entity) == Offsets.back() + Sizes.back()))
                {
                    /// If the block is continuing, then just increase it's size
                    Sizes.back() += sizeof(ECS::COMPONENTS::FMeshInstanceComponent);
                }
                else
                {
                    /// Else - push the new block...
                    Offsets.push_back(Coordinator.GetOffset<ECS::COMPONENTS::FMeshInstanceComponent>(Entity));
                    Sizes.push_back(sizeof(ECS::COMPONENTS::FMeshInstanceComponent));
                    Data.push_back(Coordinator.Data<ECS::COMPONENTS::FMeshInstanceComponent>(Entity));
                }
            }

            auto& Context = GetContext();
            Context.ResourceAllocator->LoadDataToBuffer(BLASInstanceBuffer, Sizes, Offsets, Data);

            EntitiesToUpdate.clear();

            UpdateTLAS();
        }

        void FAccelerationStructureSystem::GenerateBLAS(FEntity Entity)
        {
            auto& Coordinator = ECS::GetCoordinator();
            Coordinator.AddComponent<ECS::COMPONENTS::FAccelerationStructureComponent>(Entity, {});
            auto& AccelerationStructureComponent = GetComponent<ECS::COMPONENTS::FAccelerationStructureComponent>(Entity);

            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);
            auto& DeviceMeshComponent = GetComponent<ECS::COMPONENTS::FDeviceMeshComponent>(Entity);
            AccelerationStructureComponent =  GetContext().GenerateBlas(MESH_SYSTEM()->VertexBuffer, MESH_SYSTEM()->IndexBuffer,
                                                                        sizeof (FVertex), MeshComponent.Indexed ? MeshComponent.Indices.size() : MeshComponent.Vertices.size(),
                                                                        DeviceMeshComponent.VertexPtr, DeviceMeshComponent.IndexPtr);
        }

        FEntity FAccelerationStructureSystem::CreateInstance(FEntity Entity, const FVector3& Position)
        {
            auto& Coordinator = GetCoordinator();
            FEntity NewMeshInstance = Coordinator.CreateEntity();

            Coordinator.AddComponent<ECS::COMPONENTS::FTransformComponent>(NewMeshInstance, {Position, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f}});
            Coordinator.AddComponent<ECS::COMPONENTS::FDeviceTransformComponent>(NewMeshInstance, {});
            TRANSFORM_SYSTEM()->SyncTransform(NewMeshInstance);

            Coordinator.AddComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(NewMeshInstance, {});
            auto& DeviceRenderableComponent = RENDERABLE_SYSTEM()->GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(NewMeshInstance);
            DeviceRenderableComponent.MeshIndex = Entity;

            auto& ModelMatrix = Coordinator.GetComponent<ECS::COMPONENTS::FDeviceTransformComponent>(NewMeshInstance).ModelMatrix.Data;
            uint32_t NewIndex = AvailableIndices.front();
            AvailableIndices.pop();
            auto& BLAS = GetComponent<ECS::COMPONENTS::FAccelerationStructureComponent>(Entity);

            COMPONENTS::FMeshInstanceComponent MeshInstanceComponent{};
            MeshInstanceComponent.transform = { ModelMatrix[0].X, ModelMatrix[1].X, ModelMatrix[2].X, ModelMatrix[3].X,
                                                ModelMatrix[0].Y, ModelMatrix[1].Y, ModelMatrix[2].Y, ModelMatrix[3].Y,
                                                ModelMatrix[0].Z, ModelMatrix[1].Z, ModelMatrix[2].Z, ModelMatrix[3].Z,};
            MeshInstanceComponent.instanceCustomIndex = NewIndex;
            MeshInstanceComponent.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
            MeshInstanceComponent.mask = 0xFF;
            MeshInstanceComponent.accelerationStructureReference = GetContext().GetASDeviceAddressInfo(BLAS.AccelerationStructure);
            MeshInstanceComponent.instanceShaderBindingTableRecordOffset = 0;
            Coordinator.AddComponent<ECS::COMPONENTS::FMeshInstanceComponent>(NewMeshInstance, MeshInstanceComponent);

            EntitiesToUpdate.insert(NewMeshInstance);
            InstanceCount++;
            bIsDirty = true;
            return NewMeshInstance;
        }

        void FAccelerationStructureSystem::UpdateTLAS()
        {
            if (bIsDirty)
            {
                TLAS = GetContext().GenerateTlas(BLASInstanceBuffer, InstanceCount);
                bIsDirty = false;
            }
        }
    }
}