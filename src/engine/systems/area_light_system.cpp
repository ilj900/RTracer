#include "named_resources.h"
#include "light_component.h"
#include "device_renderable_component.h"
#include "mesh_component.h"
#include "area_light_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FAreaLightSystem::Init(uint32_t NumberOfSimultaneousSubmits)
        {
			LoadedAreaLightsCount = 0;
			CurrentAreaLightsCount = 0;

            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FAreaLightComponent) * MAX_AREA_LIGHTS,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Area_Lights");
        }

        bool FAreaLightSystem::Update()
        {
			bool bAnyUpdate = false;

			if (LoadedAreaLightsCount != CurrentAreaLightsCount)
			{
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, {sizeof(uint32_t)}, { offsetof(FUtilityData, ActiveAreaLightsCount)}, {&CurrentAreaLightsCount});
				CurrentAreaLightsCount = LoadedAreaLightsCount;
			}

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FAreaLightComponent>();
			}

			return bAnyUpdate;
        }

        bool FAreaLightSystem::Update(int Index)
        {
			bool bAnyUpdate = false;

			if (LoadedAreaLightsCount != CurrentAreaLightsCount)
			{
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, {sizeof(uint32_t)}, { offsetof(FUtilityData, ActivePointLightsCount)}, {&CurrentAreaLightsCount});
				CurrentAreaLightsCount = LoadedAreaLightsCount;
			}

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FAreaLightComponent>(Index);
			}

			return bAnyUpdate;
        }

        FEntity FAreaLightSystem::CreateAreaLightInstance(FEntity InstanceEntity)
        {
			auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(InstanceEntity);
			auto Entry = EmissiveMaterialsCount.find(RenderableComponent.MaterialIndex);

			if (Entry != EmissiveMaterialsCount.end())
			{
				if (EmissiveMaterialsCount.size() == MAX_EMISSIVE_MATERIALS)
				{
					throw std::runtime_error("Exceeded the amount of emissive material");
				}

				Entry->second++;
			}
			else
			{
				EmissiveMaterialsCount[RenderableComponent.MaterialIndex] = 1;
			}

            auto Light = COORDINATOR().CreateEntity();
            COORDINATOR().AddComponent<ECS::COMPONENTS::FAreaLightComponent>(Light, {});

            auto& AreaLightComponent = COORDINATOR().GetComponent<COMPONENTS::FAreaLightComponent>(Light);
			auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(RenderableComponent.MeshIndex);
			AreaLightComponent.IndexBufferAddress = RenderableComponent.IndexBufferAddress;
			AreaLightComponent.VertexBufferAddress = RenderableComponent.VertexBufferAddress;
			AreaLightComponent.TransformIndex = RenderableComponent.TransformIndex;
			AreaLightComponent.MeshIndex = RenderableComponent.MeshIndex;
			AreaLightComponent.MaterialIndex = RenderableComponent.MaterialIndex;
			AreaLightComponent.NumberOfTriangles = MeshComponent.Indices.size() / 3;

            MarkDirty(Light);

			CurrentAreaLightsCount++;

            return Light;
        }

		const std::unordered_map<uint32_t, uint32_t>& FAreaLightSystem::GetEmissiveMaterials()
		{
			return EmissiveMaterialsCount;
		}
    }
}
