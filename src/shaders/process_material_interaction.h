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

vec3 Transform(vec3 NormalInWorldSpace, vec3 VectorInLocalSpace)
{
	vec3 A = (abs(NormalInWorldSpace.x) > 0.0) ? vec3(0, 1, 0) : vec3(1, 0, 0);
	vec3 V = cross(NormalInWorldSpace, A);
	vec3 U = cross(NormalInWorldSpace, V);

	return VectorInLocalSpace.x * U + VectorInLocalSpace.y * NormalInWorldSpace + VectorInLocalSpace.z * V;
}

vec3 ScatterDiffuse(vec3 NormalInWorldSpace, FSamplingState SamplingState)
{
	vec2 Sample = vec2(RandomFloat(SamplingState), RandomFloat(SamplingState));

	float Phi = Sample.x * M_2_PI;
	float Theta = Sample.y * M_PI;

	vec3 Result;

	Result.z = sin(Theta) * cos(Phi);
	Result.y = cos(Theta);
	Result.x = sin(Theta) * sin(Phi);


	return normalize(Result + NormalInWorldSpace);
}

vec3 SampleMaterial(FDeviceMaterial Material, inout FRayData RayData, out uint RayType, vec3 NormalInWorldSpace, FSamplingState SamplingState, bool bFrontFacing)
{
	float LayerSample = RandomFloat(SamplingState);
	RayType = SelectLayer(Material, LayerSample);
	vec3 Color = vec3(0);

	switch (RayType)
	{
	case DIFFUSE_LAYER:
		Color = Material.BaseColor;
		vec3 ReflectionDirection = ScatterDiffuse(NormalInWorldSpace, SamplingState);
		RayData.Direction.xyz = ReflectionDirection;
		break;
	case SPECULAR_LAYER:
		Color = Material.SpecularColor;
		RayData.Direction.xyz = -reflect(-RayData.Direction.xyz, NormalInWorldSpace);
		break;
	case TRANSMISSION_LAYER:
		Color = Material.TransmissionColor;
		float EtaRatio = 0;
		if (bFrontFacing)
		{
			EtaRatio = RayData.Eta / Material.SpecularIOR;
		}
		else
		{
			EtaRatio = Material.SpecularIOR / RayData.Eta;
		}

		float NDotI = dot(NormalInWorldSpace, -RayData.Direction.xyz);
		float k = 1. - EtaRatio * EtaRatio * (1. - NDotI * NDotI);
		if (k < 0.)
		{
			RayData.Direction.xyz = -reflect(-RayData.Direction.xyz, -NormalInWorldSpace);
		}
		else
		{
			RayData.Direction.xyz = EtaRatio * -RayData.Direction.xyz - (EtaRatio * NDotI + sqrt(k)) * -NormalInWorldSpace;
		}

		RayData.Eta = Material.SpecularIOR;
		break;
	case SUBSURFACE_LAYER:
		Color = Material.SubsurfaceColor;
		break;
	case SHEEN_LAYER:
		Color = Material.SheenColor;
		break;
	case COAT_LAYER:
		Color = Material.CoatColor;
		break;
	}

	return Color;
}
#endif // PROCESS_MATERIAL_INTERACTION_H