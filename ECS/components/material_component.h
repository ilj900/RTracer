#pragma once

#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FMaterialComponent
        {
            FMaterialComponent() = default;
            FMaterialComponent(const FVector3& BaseAlbedoIn,
                               const FVector3& ReflectionAlbedoIn,
                               const FVector3& CoatingAlbedoIn,
                               const FVector3& RefractionAlbedoIn,
                               float ReflectionRoughnessIn,
                               float RefractionRoughnessIn,
                               float ReflectionIORIn,
                               float RefractionIORIn) :
                               BaseAlbedo(BaseAlbedoIn),
                               ReflectionAlbedo(ReflectionAlbedoIn),
                               CoatingAlbedo(CoatingAlbedoIn),
                               RefractionAlbedo(RefractionAlbedoIn),
                               ReflectionRoughness(ReflectionRoughnessIn),
                               RefractionRoughness(RefractionRoughnessIn),
                               ReflectionIOR(ReflectionIORIn),
                               RefractionIOR(RefractionIORIn)
                               {};

            FVector3 BaseAlbedo = FVector3(1.f, 0.f, 1.f);
            float ReflectionRoughness = 1.f;
            FVector3 ReflectionAlbedo = FVector3(0.f, 0.f, 0.f);
            float RefractionRoughness = 0.f;
            FVector3 CoatingAlbedo = FVector3(0.f, 0.f, 0.f);
            float ReflectionIOR = 0.f;
            FVector3 RefractionAlbedo = FVector3(0.f, 0.f, 0.f);
            float RefractionIOR = 0.f;

        };
    }
}