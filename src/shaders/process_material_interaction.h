#ifndef PROCESS_MATERIAL_INTERACTION_H
#define PROCESS_MATERIAL_INTERACTION_H

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
	vec3 Result = Sample3DUnitSphere(SamplingState);
	Result = normalize(Result + NormalInWorldSpace);

	return Result;
}

vec3 SampleMaterial(FDeviceMaterial Material, inout FRayData RayData, vec3 NormalInWorldSpace, inout FSamplingState SamplingState, bool bFrontFacing)
{
	float LayerSample = RandomFloat(SamplingState);
	uint RayType = SelectLayer(Material, LayerSample);
	RayData.RayFlags = 0u;
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
		RayData.Direction.xyz = reflect(RayData.Direction.xyz, NormalInWorldSpace);
		break;
	case TRANSMISSION_LAYER:
		Color = Material.TransmissionColor;
		float EtaRatio = 0;

		float R0 = (RayData.Eta - Material.SpecularIOR) / (RayData.Eta + Material.SpecularIOR);
		R0 = R0 * R0;
		/// NDotI also equals to cos(angle)
		/// Ray's direction is inverted cause it's guaranteed to face normal.
		float NDotI = dot(NormalInWorldSpace, RayData.Direction.xyz);

		float RTheta = R0 + (1. - R0) * pow(1. - abs(NDotI), 5.f);

		if (bFrontFacing)
		{
			EtaRatio = RayData.Eta / Material.SpecularIOR;
		}
		else
		{
			/// We assume that when ray leaves some medium, it always leaves into the air, thus IOR == 1.
			EtaRatio = RayData.Eta;
		}

		/// Decide on whether the ray is reflected or refracted
		if (RandomFloat(SamplingState) < RTheta)
		{
			RayData.Direction.xyz = reflect(RayData.Direction.xyz, NormalInWorldSpace);
			RayType = SPECULAR_LAYER;
		}
		else
		{
			float k = 1. - EtaRatio * EtaRatio * (1. - NDotI * NDotI);

			if (k < 0. || RandomFloat(SamplingState) < RTheta)
			{
				RayData.Direction.xyz = reflect(RayData.Direction.xyz, NormalInWorldSpace);
				RayType = SPECULAR_LAYER;
			}
			else
			{
				RayData.Direction.xyz = EtaRatio * RayData.Direction.xyz - (EtaRatio * NDotI + sqrt(k)) * NormalInWorldSpace;
			}
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

	RayData.RayFlags |= RayType;
	return Color;
}
#endif // PROCESS_MATERIAL_INTERACTION_H