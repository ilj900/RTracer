#ifndef PROCESS_MATERIAL_INTERACTION_H
#define PROCESS_MATERIAL_INTERACTION_H

#define DIFFUSE_LAYER 			1u
#define SPECULAR_LAYER 			1u << 1
#define TRANSMISSION_LAYER 		1u << 2
#define SUBSURFACE_LAYER		1u << 3
#define SHEEN_LAYER 			1u << 4
#define COAT_LAYER 				1u << 5

uint SelectLayer(FDeviceMaterial Material, float MaterialSample)
{
	float TotalWeight = Material.BaseWeight + Material.SpecularWeight + Material.TransmissionWeight + Material.SubsurfaceWeight + Material.SheenWeight + Material.CoatWeight;
	float Weight = 0;
	MaterialSample *= TotalWeight;

	Weight += Material.BaseWeight;
	if (MaterialSample <= Weight)
		return DIFFUSE_LAYER;

	Weight += Material.SpecularWeight;
	if (MaterialSample <= Weight)
		return SPECULAR_LAYER;

	Weight += Material.TransmissionWeight;
	if (MaterialSample <= Weight)
		return TRANSMISSION_LAYER;

	Weight += Material.SubsurfaceWeight;
	if (MaterialSample <= Weight)
		return SUBSURFACE_LAYER;

	Weight += Material.SheenWeight;
	if (MaterialSample <= Weight)
		return SHEEN_LAYER;

	return COAT_LAYER;
}

vec3 SampleMaterial(FDeviceMaterial Material, inout FRayData RayData, vec3 NormalInWorldSpace, float MaterialSample)
{
	uint Layer = SelectLayer(Material);
	vec3 Color = vec3(0);

	switch (Layer)
	{
	case DIFFUSE_LAYER:
		Color += Material.BaseColor;
		RayData.Direction.xyz = -reflect(-RayData.Direction.xyz, NormalInWorldSpace);
		break;
	case SPECULAR_LAYER:
		RayData.Direction.xyz = -reflect(-RayData.Direction.xyz, NormalInWorldSpace);
		break;
	case TRANSMISSION_LAYER:
		break;
	case SUBSURFACE_LAYER:
		break;
	case SHEEN_LAYER:
		break;
	case COAT_LAYER:
		break;
	}

	return Color;
}
#endif // PROCESS_MATERIAL_INTERACTION_H