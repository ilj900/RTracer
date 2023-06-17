#include "components/light_component.h"
#include "systems/light_system.h"
#include "coordinator.h"

#include "vk_context.h"

#include <cassert>

namespace ECS
{
    namespace SYSTEMS
    {
        void FLightSystem::Init(int NumberOfSimultaneousSubmits)
        {
            this->NumberOfSimultaneousSubmits = NumberOfSimultaneousSubmits;
            auto& Coordinator = GetCoordinator();
            auto& Context = GetContext();
            auto LightComponentsData = Coordinator.Data<ECS::COMPONENTS::FLightComponent>();
            auto LightComponentsSize = Coordinator.Size<ECS::COMPONENTS::FLightComponent>();

            VkDeviceSize TransformBufferSize = LightComponentsSize * NumberOfSimultaneousSubmits;

            DeviceLightBuffer = GetContext().CreateBuffer(TransformBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Device_Transform_Buffer");

            for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                Context.ResourceAllocator->LoadDataToBuffer(DeviceLightBuffer, LightComponentsSize, LightComponentsSize * i, LightComponentsData);
            }

            BufferPartThatNeedsUpdate.resize(NumberOfSimultaneousSubmits);
        }

        void FLightSystem::Update()
        {
            for (int i = 0; i < BufferPartThatNeedsUpdate.size(); ++i)
            {
                if (true == BufferPartThatNeedsUpdate[i])
                {
                    auto& Coordinator = GetCoordinator();
                    auto& Context = GetContext();
                    auto LightComponentsData = Coordinator.Data<ECS::COMPONENTS::FLightComponent>();
                    auto LightComponentsSize = Coordinator.Size<ECS::COMPONENTS::FLightComponent>();

                    Context.ResourceAllocator->LoadDataToBuffer(DeviceLightBuffer,
                                                                LightComponentsSize,
                                                                LightComponentsSize * i,
                                                                LightComponentsData);
                    BufferPartThatNeedsUpdate[i] = false;
                }
            }
        }

        FLightSystem& FLightSystem::SetLightPosition(FEntity LightEntity, float X, float Y, float Z)
        {
            auto& Coordinator = GetCoordinator();
            auto& LightComponent = Coordinator.GetComponent<COMPONENTS::FLightComponent>(LightEntity);
            LightComponent.Position = FVector3(X, Y, Z);

            return *this;
        }

        FLightSystem& FLightSystem::SetLightPosition(FEntity LightEntity, const FVector3& Position)
        {
            auto& Coordinator = GetCoordinator();
            auto& LightComponent = Coordinator.GetComponent<COMPONENTS::FLightComponent>(LightEntity);
            LightComponent.Position = Position;

            return *this;
        }

        FLightSystem& FLightSystem::SetLightDirection(FEntity LightEntity, const FVector3& Direction)
        {
            auto& Coordinator = GetCoordinator();
            auto& LightComponent = Coordinator.GetComponent<COMPONENTS::FLightComponent>(LightEntity);
            LightComponent.Direction = Direction;

            return *this;
        }

        FLightSystem& FLightSystem::SetLightColor(FEntity LightEntity, const FVector3& Color)
        {
            auto& Coordinator = GetCoordinator();
            auto& LightComponent = Coordinator.GetComponent<COMPONENTS::FLightComponent>(LightEntity);
            LightComponent.Color = Color;

            return *this;
        }

        FEntity FLightSystem::CreatePointLight(const FVector3& Position, const FVector3& Color, float Intensity)
        {
            auto& Coordinator = GetCoordinator();
            auto Light = Coordinator.CreateEntity();
            Coordinator.AddComponent<ECS::COMPONENTS::FLightComponent>(Light, {});

            auto& LightComponent = Coordinator.GetComponent<COMPONENTS::FLightComponent>(Light);
            LightComponent.Type = LIGHT_TYPE::POINT_LIGHT;
            LightComponent.Position = Position;
            LightComponent.Color = Color;
            LightComponent.Intensity = Intensity;

            return Light;
        }

        FEntity FLightSystem::CreateDirectionalLight(const FVector3& Direction, const FVector3& Color, float Intensity)
        {
            auto& Coordinator = GetCoordinator();
            auto Light = Coordinator.CreateEntity();
            Coordinator.AddComponent<ECS::COMPONENTS::FLightComponent>(Light, {});

            auto& LightComponent = Coordinator.GetComponent<COMPONENTS::FLightComponent>(Light);
            LightComponent.Type = LIGHT_TYPE::DIRECTIONAL_LIGHT;
            LightComponent.Direction = Direction;
            LightComponent.Color = Color;
            LightComponent.Intensity = Intensity;

            return Light;
        }

        FEntity FLightSystem::CreateSpotLight(const FVector3& Position, const FVector3& Color, float Intensity, float OuterAngle, float InnerAngle)
        {
            auto& Coordinator = GetCoordinator();
            auto Light = Coordinator.CreateEntity();
            Coordinator.AddComponent<ECS::COMPONENTS::FLightComponent>(Light, {});

            auto& LightComponent = Coordinator.GetComponent<COMPONENTS::FLightComponent>(Light);
            LightComponent.Type = LIGHT_TYPE::SPOT_LIGHT;
            LightComponent.Position = Position;
            LightComponent.Color = Color;
            LightComponent.Intensity = Intensity;
            LightComponent.OuterAngle = OuterAngle;
            LightComponent.InnerAngle = InnerAngle;

            return Light;
        }


        void FLightSystem::RequestAllUpdate()
        {
            for(int i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                BufferPartThatNeedsUpdate[i] = true;
            }
        }

        void FLightSystem::RequestUpdate(int FrameIndex)
        {
            BufferPartThatNeedsUpdate[FrameIndex] = true;
        }

        int FLightSystem::GetTotalSize()
        {
            auto& Coordinator = GetCoordinator();
            return Coordinator.Size<ECS::COMPONENTS::FLightComponent>();
        }

    }
}