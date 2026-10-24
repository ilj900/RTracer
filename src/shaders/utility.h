#ifndef UTILITY_H
#define UTILITY_H

vec4 IndexToColor(uint Value)
{
    uint ColorHash = MurMur3_32(Value, 0);
    vec4 Color = vec4(0);
    Color.x = ((ColorHash >> 24) & 0xFFu) / 255.;
    Color.y = ((ColorHash >> 18) & 0xFFu) / 255.;
    Color.z = ((ColorHash >> 12) & 0xFFu) / 255.;
    return Color;
}

vec3 ApplyGamma(vec3 Color, float Gamma)
{
	return pow(Color, vec3(1.f / Gamma));
}

vec3 RemoveGamma(vec3 Color, float Gamma)
{
	return pow(Color, vec3(Gamma));
}

void SaveAOVs(uvec2 PixelCoords, vec3 ShadingNormal, vec3 GeometricNormal, vec2 UV, vec3 WorldSpacePosition, float Opacity, float Depth, FOpenPBRSurface Material,
              vec3 Luminance, uint RenderableIndex, uint PrimitiveIndex, uint Materialindex)
{
	vec4 ValueToSave = vec4(0);

	if (PushConstants.BounceIndex != 0)
	{
		ValueToSave = imageLoad(AOV_RGBA32F_Image, ivec2(PixelCoords)).xyzw;
	}

    switch (UtilityData.AOVIndex)
    {
        case AOV_SHADING_NORMAL: { ValueToSave += vec4((ShadingNormal + vec3(1)) * vec3(0.5), 0); break; }
        case AOV_GEOMETRIC_NORMAL: { ValueToSave += vec4((GeometricNormal + vec3(1)) * vec3(0.5), 0); break; }
        case AOV_UV_COORDS: { ValueToSave += vec4(UV, 0, 0); break; }
        case AOV_WORLD_SPACE_POSITION: { ValueToSave += vec4(WorldSpacePosition, 0); break; }
        case AOV_OPACITY: { ValueToSave += vec4(Opacity, Opacity, Opacity, 0); break; }
        case AOV_DEPTH: { ValueToSave += vec4(Depth, Depth, Depth, 0); break; }
        // Base
        case AOV_MATERIAL_BASE_WEIGHT: { ValueToSave += vec4(vec3(Material.BaseWeight), 0); break; }
        case AOV_MATERIAL_BASE_COLOR: { ValueToSave += vec4(Material.BaseColor, 0); break; }
        case AOV_MATERIAL_BASE_METALNESS: { ValueToSave += vec4(vec3(Material.BaseMetalness), 0); break; }
        case AOV_MATERIAL_BASE_DIFFUSE_ROUGHNESS: { ValueToSave += vec4(vec3(Material.BaseDiffuseRoughness), 0); break; }
        // Specular
        case AOV_MATERIAL_SPECULAR_WEIGHT: { ValueToSave += vec4(vec3(Material.SpecularWeight), 0); break; }
        case AOV_MATERIAL_SPECULAR_COLOR: { ValueToSave += vec4(Material.SpecularColor, 0); break; }
        case AOV_MATERIAL_SPECULAR_ROUGHNESS: { ValueToSave += vec4(vec3(Material.SpecularRoughness), 0); break; }
        case AOV_MATERIAL_SPECULAR_ROUGHNESS_ANISOTROPY: { ValueToSave += vec4(vec3(Material.SpecularRoughnessAnisotropy), 0); break; }
        case AOV_MATERIAL_SPECULAR_IOR: { ValueToSave += vec4(vec3(Material.SpecularIor), 0); break; }
        // Transmission
        case AOV_MATERIAL_TRANSMISSION_WEIGHT: { ValueToSave += vec4(vec3(Material.TransmissionWeight), 0); break; }
        case AOV_MATERIAL_TRANSMISSION_COLOR: { ValueToSave += vec4(Material.TransmissionColor, 0); break; }
        case AOV_MATERIAL_TRANSMISSION_DEPTH: { ValueToSave += vec4(vec3(Material.TransmissionDepth), 0); break; }
        case AOV_MATERIAL_TRANSMISSION_SCATTER: { ValueToSave += vec4(Material.TransmissionScatter, 0); break; }
        case AOV_MATERIAL_TRANSMISSION_SCATTER_ANISOTROPY: { ValueToSave += vec4(vec3(Material.TransmissionScatterAnisotropy), 0); break; }
        case AOV_MATERIAL_TRANSMISSION_DISPERSION_SCALE: { ValueToSave += vec4(vec3(Material.TransmissionDispersionScale), 0); break; }
        case AOV_MATERIAL_TRANSMISSION_DISPERSION_ABBE_NUMBER: { ValueToSave += vec4(vec3(Material.TransmissionDispersionAbbeNumber), 0); break; }
        // Subsurface
        case AOV_MATERIAL_SUBSURFACE_WEIGHT: { ValueToSave += vec4(vec3(Material.SubsurfaceWeight), 0); break; }
        case AOV_MATERIAL_SUBSURFACE_COLOR: { ValueToSave += vec4(Material.SubsurfaceColor, 0); break; }
        case AOV_MATERIAL_SUBSURFACE_RADIUS: { ValueToSave += vec4(vec3(Material.SubsurfaceRadius), 0); break; }
        case AOV_MATERIAL_SUBSURFACE_RADIUS_SCALE: { ValueToSave += vec4(Material.SubsurfaceRadiusScale, 0); break; }
        case AOV_MATERIAL_SUBSURFACE_SCATTER_ANISOTROPY: { ValueToSave += vec4(vec3(Material.SubsurfaceScatterAnisotropy), 0); break; }
        // Coat
        case AOV_MATERIAL_COAT_WEIGHT: { ValueToSave += vec4(vec3(Material.CoatWeight), 0); break; }
        case AOV_MATERIAL_COAT_COLOR: { ValueToSave += vec4(Material.CoatColor, 0); break; }
        case AOV_MATERIAL_COAT_ROUGHNESS: { ValueToSave += vec4(vec3(Material.CoatRoughness), 0); break; }
        case AOV_MATERIAL_COAT_ROUGHNESS_ANISOTROPY: { ValueToSave += vec4(vec3(Material.CoatRoughnessAnisotropy), 0); break; }
        case AOV_MATERIAL_COAT_IOR: { ValueToSave += vec4(vec3(Material.CoatIor), 0); break; }
        case AOV_MATERIAL_COAT_DARKENING: { ValueToSave += vec4(vec3(Material.CoatDarkening), 0); break; }
        // Fuzz
        case AOV_MATERIAL_FUZZ_WEIGHT: { ValueToSave += vec4(vec3(Material.FuzzWeight), 0); break; }
        case AOV_MATERIAL_FUZZ_COLOR: { ValueToSave += vec4(Material.FuzzColor, 0); break; }
        case AOV_MATERIAL_FUZZ_ROUGHNESS: { ValueToSave += vec4(vec3(Material.FuzzRoughness), 0); break; }
        // Emission
        case AOV_MATERIAL_EMISSION_LUMINANCE: { ValueToSave += vec4(vec3(Material.EmissionLuminance), 0); break; }
        case AOV_MATERIAL_EMISSION_COLOR: { ValueToSave += vec4(Material.EmissionColor, 0); break; }
        // Thin film
        case AOV_MATERIAL_THIN_FILM_WEIGHT: { ValueToSave += vec4(vec3(Material.ThinFilmWeight), 0); break; }
        case AOV_MATERIAL_THIN_FILM_THICKNESS: { ValueToSave += vec4(vec3(Material.ThinFilmThickness), 0); break; }
        case AOV_MATERIAL_THIN_FILM_IOR: { ValueToSave += vec4(vec3(Material.ThinFilmIor), 0); break; }
          // Geometry
        case AOV_MATERIAL_GEOMETRY_OPACITY: { ValueToSave += vec4(vec3(Material.GeometryOpacity), 0); break; }
        case AOV_MATERIAL_GEOMETRY_THIN_WALLED: { ValueToSave += vec4(vec3(Material.GeometryThinWalled), 0); break; }
        case AOV_MATERIAL_GEOMETRY_NORMAL: { ValueToSave += vec4(Material.GeometryNormal, 0); break; }
        case AOV_MATERIAL_GEOMETRY_TANGENT: { ValueToSave += vec4(Material.GeometryTangent, 0); break; }
        case AOV_MATERIAL_GEOMETRY_COAT_NORMAL: { ValueToSave += vec4(Material.GeometryCoatNormal, 0); break; }
        case AOV_MATERIAL_GEOMETRY_COAT_TANGENT: { ValueToSave += vec4(Material.GeometryCoatTangent, 0); break; }
        // Misc
        case AOV_LUMINANCE: { ValueToSave += vec4(Luminance, 0); break; }
        case AOV_BOUNCE_HEATMAP: { ValueToSave = vec4(mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), float(PushConstants.BounceIndex - 1) / float(LAST_BOUNCE)), 0); break; }
        case AOV_RENDERABLE_INDEX: { ValueToSave += IndexToColor(RenderableIndex); break; }
        case AOV_PRIMITIVE_INDEX: { ValueToSave += IndexToColor(PrimitiveIndex); break; }
        case AOV_MATERIAL_INDEX: { ValueToSave += IndexToColor(Materialindex); break; }
        case AOV_DEBUG_LAYER_0: { ValueToSave += DebugGlobal0; break; }
        case AOV_DEBUG_LAYER_1: { ValueToSave += DebugGlobal1; break; }
        case AOV_DEBUG_LAYER_2: { ValueToSave += DebugGlobal2; break; }
        case AOV_DEBUG_LAYER_3: { ValueToSave += DebugGlobal3; break; }
        case AOV_DEBUG_LAYER_4: { ValueToSave += DebugGlobal4; break; }
        case AOV_DEBUG_LAYER_5: { ValueToSave += DebugGlobal5; break; }
        case AOV_DEBUG_LAYER_6: { ValueToSave += DebugGlobal6; break; }
        case AOV_DEBUG_LAYER_7: { ValueToSave += DebugGlobal7; break; }
    }

	imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), ValueToSave);
}

#endif // UTILITY_H
