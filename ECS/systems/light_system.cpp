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
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FLightComponent) * 100, "");
        }

        void FLightSystem::Update()
        {
            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FLightComponent>();
        }

        void FLightSystem::Update(int Index)
        {
            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FLightComponent>(Index);
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

        FLightSystem& FLightSystem::SetLightIntensity(FEntity LightEntity, float Intensity)
        {
            auto& Coordinator = GetCoordinator();
            auto& LightComponent = Coordinator.GetComponent<COMPONENTS::FLightComponent>(LightEntity);
            LightComponent.Intensity = Intensity;

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
    }
}