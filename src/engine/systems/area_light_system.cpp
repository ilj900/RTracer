#include "named_resources.h"
#include "light_component.h"
#include "device_renderable_component.h"
#include "device_transform_component.h"
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

			if (bAreaLightAddressTableShouldBeUpdated)
			{
				auto [UpdatedAliasTable, _] = GenerateImportanceMapFast<ECS::COMPONENTS::FAreaLightComponent>(COORDINATOR().Data<ECS::COMPONENTS::FAreaLightComponent>(),
					CurrentAreaLightsCount, 1, [](ECS::COMPONENTS::FAreaLightComponent Component){return double(Component.Area);});

				RESOURCE_ALLOCATOR()->LoadDataToBuffer(AREA_LIGHTS_IMPORTANCE_BUFFER, UpdatedAliasTable.size() * sizeof(FAliasTableEntry), 0, UpdatedAliasTable.data());

				bAreaLightAddressTableShouldBeUpdated = false;
			}

			if (LoadedAreaLightsCount != CurrentAreaLightsCount ||
				LoadedAreaLightArea != CurrentAreaLightArea)
			{
				/// Load two entries even though only one can be dirty
				/// Pay close attention to the order of member fields: CurrentAreaLightsCount should be followed by CurrentAreaLightArea
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, sizeof(uint32_t) * 2,  offsetof(FUtilityData, ActiveAreaLightsCount), &CurrentAreaLightsCount);
				LoadedAreaLightsCount = CurrentAreaLightsCount;
				LoadedAreaLightArea = CurrentAreaLightArea;
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

			if (bAreaLightAddressTableShouldBeUpdated)
			{
				auto [UpdatedAliasTable, _] = GenerateImportanceMapFast<ECS::COMPONENTS::FAreaLightComponent>(COORDINATOR().Data<ECS::COMPONENTS::FAreaLightComponent>(),
					CurrentAreaLightsCount, 1, [](ECS::COMPONENTS::FAreaLightComponent Component){return double(Component.Area);});

				RESOURCE_ALLOCATOR()->LoadDataToBuffer(AREA_LIGHTS_IMPORTANCE_BUFFER, UpdatedAliasTable.size() * sizeof(FAliasTableEntry), 0, UpdatedAliasTable.data());

				bAreaLightAddressTableShouldBeUpdated = false;
			}

			if (LoadedAreaLightsCount != CurrentAreaLightsCount ||
				LoadedAreaLightArea != CurrentAreaLightArea)
			{
				/// Load two entries even though only one can be dirty
				/// Pay close attention to the order of member fields: CurrentAreaLightsCount should be followed by CurrentAreaLightArea
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, sizeof(uint32_t) * 2,  offsetof(FUtilityData, ActiveAreaLightsCount), &CurrentAreaLightsCount);
				LoadedAreaLightsCount = CurrentAreaLightsCount;
				LoadedAreaLightArea = CurrentAreaLightArea;
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
			AreaLightComponent.MaterialIndex = RenderableComponent.MaterialIndex;
			/// We need to compute the area of the area light
			/// It's the area of the mesh multiplied by scale of the model matrix of that mesh
			auto Transform = GetComponent<ECS::COMPONENTS::FDeviceTransformComponent>(InstanceEntity);
			/// We have to compute the new area og the mesh because it's transform changed it area;
			AreaLightComponent.Area = MeshComponent.ComputeArea(Transform.ModelMatrix);

			/// We pack renderable index and IsIndex flag in one field
			uint32_t IsIndexedFlagAndRenderableIndex = MeshComponent.Indices.empty() ? 0 : 1u << 31;
			IsIndexedFlagAndRenderableIndex |= RenderableComponent.RenderableIndex;
			AreaLightComponent.IsIndexedFlagAndRenderableIndex = IsIndexedFlagAndRenderableIndex;
			AreaLightComponent.NumberOfTriangles = MeshComponent.Indices.empty() ? (MeshComponent.Vertices.size() / 3) : (MeshComponent.Indices.size() / 3);

			std::vector<float> Areas(AreaLightComponent.NumberOfTriangles, 0);

			if (MeshComponent.Indices.empty())
			{
				for (int i = 0; i < MeshComponent.Vertices.size(); i += 3)
				{
					Areas[i / 3] = 0.5f * Cross(MeshComponent.Vertices[i+1].Position - MeshComponent.Vertices[i].Position, MeshComponent.Vertices[i+2].Position - MeshComponent.Vertices[i].Position).Length();
				}
			}
			else
			{
				for (int i = 0; i < MeshComponent.Indices.size(); i += 3)
				{
					uint32_t I0 = MeshComponent.Indices[i];
					uint32_t I1 = MeshComponent.Indices[i + 1];
					uint32_t I2 = MeshComponent.Indices[i + 2];
					Areas[i / 3] = 0.5f * Cross(MeshComponent.Vertices[I1].Position - MeshComponent.Vertices[I0].Position, MeshComponent.Vertices[I2].Position - MeshComponent.Vertices[I0].Position).Length();
				}
			}

			auto [AreaLightAliasTable, _] = GenerateImportanceMapFast<float>(Areas.data(),
				AreaLightComponent.NumberOfTriangles, 1, [](float Component){return double(Component);});

			std::string BufferName = "AreaLight_" + std::to_string(Light) + "_AliasTableBuffer";
			FBuffer AreaLightAliasTableBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(AreaLightAliasTable.size() * sizeof(FAliasTableEntry),
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, BufferName);
			RESOURCE_ALLOCATOR()->RegisterBuffer(AreaLightAliasTableBuffer, BufferName);

			RESOURCE_ALLOCATOR()->LoadDataToBuffer(BufferName, AreaLightAliasTable.size() * sizeof(FAliasTableEntry), 0, AreaLightAliasTable.data());

			AreaLightComponent.AliasTableBufferAddress = VK_CONTEXT()->GetBufferDeviceAddressInfo(RESOURCE_ALLOCATOR()->GetBuffer(BufferName));

            MarkDirty(Light);

			CurrentAreaLightsCount++;
			CurrentAreaLightArea += AreaLightComponent.Area;
			bAreaLightAddressTableShouldBeUpdated = true;

            return Light;
        }

		const std::unordered_map<uint32_t, uint32_t>& FAreaLightSystem::GetEmissiveMaterials()
		{
			return EmissiveMaterialsCount;
		}
    }
}
