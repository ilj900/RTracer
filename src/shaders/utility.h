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

void SaveAOVs(uvec2 PixelCoords, vec3 ShadingNormal, vec3 GeometricNormal, vec2 UV, vec3 WorldSpacePosition, float Opacity, float Depth, FDeviceMaterial Material,
              vec3 Luminance, uint RenderableIndex, uint PrimitiveIndex, uint Materialindex, vec4 DebugData0, vec4 DebugData1, vec4 DebugData2, vec4 DebugData3)
{
    switch (UtilityData.AOVIndex)
    {
        case AOV_SHADING_NORMAL: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4((ShadingNormal + vec3(1)) * vec3(0.5), 0)); break; }
        case AOV_GEOMETRIC_NORMAL: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4((GeometricNormal + vec3(1)) * vec3(0.5), 0)); break; }
        case AOV_UV_COORDS: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(UV, 0, 0)); break; }
        case AOV_WORLD_SPACE_POSITION: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(WorldSpacePosition, 0)); break; }
        case AOV_OPACITY: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Opacity, Opacity, Opacity, 0)); break; }
        case AOV_DEPTH: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Depth, Depth, Depth, 0)); break; }
        case AOV_MATERIAL_ALBEDO_WEIGHT: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.BaseWeight), 0)); break; }
    	case AOV_MATERIAL_ALBEDO: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Material.BaseColor, 0)); break; }
    	case AOV_MATERIAL_DIFFUSE_ROUGHNESS: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.DiffuseRoughness), 0)); break; }
    	case AOV_MATERIAL_METALNESS: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.Metalness), 0)); break; }
    	case AOV_MATERIAL_NORMAL: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Material.Normal, 0)); break; }
    	case AOV_MATERIAL_SPECULAR_WEIGHT: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.SpecularWeight), 0)); break; }
    	case AOV_MATERIAL_SPECULAR_COLOR: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Material.SpecularColor, 0)); break; }
    	case AOV_MATERIAL_SPECULAR_ROUGHNESS: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.SpecularRoughness), 0)); break; }
    	case AOV_MATERIAL_SPECULAR_IOR: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.SpecularIOR), 0)); break; }
    	case AOV_MATERIAL_SPECULAR_ANISOTROPY: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.SpecularAnisotropy), 0)); break; }
    	case AOV_MATERIAL_SPECULAR_ROTATION: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.SpecularRotation), 0)); break; }
    	case AOV_MATERIAL_TRANSMISSION_WEIGHT: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.TransmissionWeight), 0)); break; }
    	case AOV_MATERIAL_TRANSMISSION_COLOR: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Material.TransmissionColor, 0)); break; }
    	case AOV_MATERIAL_TRANSMISSION_DEPTH: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.TransmissionDepth), 0)); break; }
    	case AOV_MATERIAL_TRANSMISSION_SCATTER: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Material.TransmissionScatter, 0)); break; }
    	case AOV_MATERIAL_TRANSMISSION_ANISOTROPY: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.TransmissionAnisotropy), 0)); break; }
    	case AOV_MATERIAL_TRANSMISSION_DISPERSION: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.TransmissionDispersion), 0)); break; }
    	case AOV_MATERIAL_TRANSMISSION_ROUGHNESS: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.TransmissionRoughness), 0)); break; }
    	case AOV_MATERIAL_SUBSURFACE_WEIGHT: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.SubsurfaceWeight), 0)); break; }
    	case AOV_MATERIAL_SUBSURFACE_COLOR: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Material.SubsurfaceColor, 0)); break; }
    	case AOV_MATERIAL_SUBSURFACE_RADIUS: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Material.SubsurfaceRadius, 0)); break; }
    	case AOV_MATERIAL_SUBSURFACE_SCALE: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.SubsurfaceScale), 0)); break; }
    	case AOV_MATERIAL_SUBSURFACE_ANISOTROPY: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.SubsurfaceAnisotropy), 0)); break; }
    	case AOV_MATERIAL_SHEEN_WEIGHT: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.SheenWeight), 0)); break; }
    	case AOV_MATERIAL_SHEEN_COLOR: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Material.SheenColor, 0)); break; }
    	case AOV_MATERIAL_SHEEN_ROUGHNESS: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.SheenRoughness), 0)); break; }
    	case AOV_MATERIAL_COAT_WEIGHT: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.CoatWeight), 0)); break; }
    	case AOV_MATERIAL_COAT_COLOR: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Material.CoatColor, 0)); break; }
    	case AOV_MATERIAL_COAT_ROUGHNESS: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.CoatRoughness), 0)); break; }
    	case AOV_MATERIAL_COAT_ANISOTROPY: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.CoatAnisotropy), 0)); break; }
    	case AOV_MATERIAL_COAT_ROTATION: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.CoatRotation), 0)); break; }
    	case AOV_MATERIAL_COAT_IOR: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.CoatIOR), 0)); break; }
    	case AOV_MATERIAL_COAT_NORMAL: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Material.CoatNormal, 0)); break; }
    	case AOV_MATERIAL_COAT_AFFECT_COLOR: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.CoatAffectColor), 0)); break; }
    	case AOV_MATERIAL_COAT_AFFECT_ROUGHNESS: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.CoatAffectRoughness), 0)); break; }
    	case AOV_MATERIAL_THIN_FILM_THICKNESS: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.ThinFilmThickness), 0)); break; }
    	case AOV_MATERIAL_THIN_FILM_IOR: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.ThinFilmIOR), 0)); break; }
    	case AOV_MATERIAL_EMISSION_WEIGHT: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.EmissionWeight), 0)); break; }
    	case AOV_MATERIAL_COLOR: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Material.EmissionColor, 0)); break; }
    	case AOV_MATERIAL_OPACITY: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.Opacity), 0)); break; }
    	case AOV_MATERIAL_THIN_WALLED: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(vec3(Material.ThinWalled), 0)); break; }
        case AOV_LUMINANCE: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), vec4(Luminance, 0)); break; }
        case AOV_RENDERABLE_INDEX: { vec4 RenderableIdColor = IndexToColor(RenderableIndex); imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), RenderableIdColor); break; }
        case AOV_PRIMITIVE_INDEX: { vec4 PrimitiveIdColor = IndexToColor(PrimitiveIndex); imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), PrimitiveIdColor); break; }
        case AOV_MATERIAL_INDEX: { vec4 MaterialIdColor = IndexToColor(Materialindex); imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), MaterialIdColor); break; }
        case AOV_DEBUG_LAYER_0: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), DebugData0); break; }
        case AOV_DEBUG_LAYER_1: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), DebugData1); break; }
        case AOV_DEBUG_LAYER_2: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), DebugData2); break; }
        case AOV_DEBUG_LAYER_3: { imageStore(AOV_RGBA32F_Image, ivec2(PixelCoords), DebugData3); break; }
    }
}

#endif // UTILITY_H