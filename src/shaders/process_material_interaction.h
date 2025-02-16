#ifndef PROCESS_MATERIAL_INTERACTION_H
#define PROCESS_MATERIAL_INTERACTION_H

uint SelectLayer(FDeviceMaterial Material, float MaterialSample)
{
	float TotalWeight = Material.BaseWeight + Material.SpecularWeight + Material.TransmissionWeight + Material.SubsurfaceWeight + Material.SheenWeight + Material.CoatWeight  + Material.EmissionWeight;
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

	Weight += Material.CoatWeight;
	if (MaterialSample <= Weight)
		return COAT_LAYER;

	return EMISSION_LAYER;
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
		vec2 RandomSquare = Sample2DUnitQuad(SamplingState);
		mat3 TNBMatrix = CreateTNBMatrix(NormalInWorldSpace);
		vec3 LocalViewDirection = RayData.Direction.xyz * transpose(TNBMatrix);
		vec3 NewNormal = SampleGGXVNDF(-RayData.Direction.xyz, Material.SpecularRoughness, Material.SpecularRoughness, RandomSquare.x, RandomSquare.y) * TNBMatrix;
		if (b)
		{
			debugPrintfEXT("((0, 0, 0), (%f, %f, %f), 'r'),  # NormalInWorldSpace\n", NormalInWorldSpace.x, NormalInWorldSpace.y, NormalInWorldSpace.z);
			debugPrintfEXT("((0, 0, 0), (%f, %f, %f), 'g'),  # NewNormal generated\n", NewNormal.x, NewNormal.y, NewNormal.z);
		}
		NewNormal = normalize(NewNormal * transpose(TNBMatrix));
		if (b)
		{
			debugPrintfEXT("((0, 0, 0), (%f, %f, %f), 'b'),  # NewNormal in world space\n", NewNormal.x, NewNormal.y, NewNormal.z);
		}
		float CosAngle = dot(NewNormal, RayData.Direction.xyz);
		if (b)
		{
			debugPrintfEXT("CosAngle %f\n", CosAngle);
		}
		if (CosAngle > 0.)
		{
			NewNormal = reflect(-NewNormal, NormalInWorldSpace);
			if (b)
			{
				debugPrintfEXT("Had to reflectNormal\n");
			}
		}
		if (b)
		{
			debugPrintfEXT("((%f, %f, %f), (0, 0, 0), 'c'),  # RayData.Direction before\n", -RayData.Direction.x, -RayData.Direction.y, -RayData.Direction.z);
		}
		RayData.Direction.xyz = reflect(RayData.Direction.xyz, NewNormal);
		if (b)
		{
			debugPrintfEXT("((0, 0, 0), (%f, %f, %f), 'm'),  # NewNormal final\n", NewNormal.x, NewNormal.y, NewNormal.z);
			debugPrintfEXT("((0, 0, 0), (%f, %f, %f), 'y'),  # RayData.Direction after\n", RayData.Direction.x, RayData.Direction.y, RayData.Direction.z);
		}
		break;
	case TRANSMISSION_LAYER:
		Color = Material.TransmissionColor;
		float IOR1 = bFrontFacing ? RayData.Eta : Material.SpecularIOR;
		float IOR2 = bFrontFacing ? Material.SpecularIOR : 1;
		float EtaRatio = IOR1 / IOR2;

		float R0 = (IOR1 - IOR2) / (IOR1 + IOR2);
		R0 *= R0;
		/// NDotI also equals to cos(angle)
		/// Ray's direction is inverted cause it's guaranteed to face normal.
		float NDotI = dot(NormalInWorldSpace, RayData.Direction.xyz);

		float RTheta = R0 + (1. - R0) * pow(1. - abs(NDotI), 5.f);

		/// Decide on whether the ray is reflected or refracted
		float RF = RandomFloat(SamplingState);
		if (RF < RTheta)
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
				Color *= EtaRatio * EtaRatio;
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
	case EMISSION_LAYER:
		Color = Material.EmissionColor;
		break;
	}

	RayData.RayFlags |= RayType;
	return Color;
}
#endif // PROCESS_MATERIAL_INTERACTION_H