#pragma once

#include "maths.h"
#include "common_structures.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FMaterialComponent : FMaterial
        {
            FMaterialComponent()
            {
                BaseAlbedo = FVector3(1.f, 0.f, 1.f);
                ReflectionRoughness = 1.f;
                ReflectionAlbedo = FVector3(0.f, 0.f, 0.f);
                RefractionRoughness = 0.f;
                CoatingAlbedo = FVector3(0.f, 0.f, 0.f);
                ReflectionIOR = 0.f;
                RefractionAlbedo = FVector3(0.f, 0.f, 0.f);
                RefractionIOR = 0.f;
            };

            FMaterialComponent(const FVector3& BaseAlbedoIn, const FVector3& ReflectionAlbedoIn, const FVector3& CoatingAlbedoIn, const FVector3& RefractionAlbedoIn, float ReflectionRoughnessIn, float RefractionRoughnessIn, float ReflectionIORIn, float RefractionIORIn)
            {
                BaseAlbedo = BaseAlbedoIn;
                ReflectionAlbedo = ReflectionAlbedoIn;
                CoatingAlbedo = CoatingAlbedoIn;
                RefractionAlbedo = RefractionAlbedoIn;
                ReflectionRoughness = ReflectionRoughnessIn;
                RefractionRoughness = RefractionRoughnessIn;
                ReflectionIOR = ReflectionIORIn;
                RefractionIOR = RefractionIORIn;
            };

        };
    }
}