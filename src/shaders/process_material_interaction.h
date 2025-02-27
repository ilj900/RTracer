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

/// Generates a cosine weighted direction in tangent-space
vec3 ScatterOrenNayar(FSamplingState SamplingState)
{
	return SampleCosineHemisphere(SamplingState);
}

vec3 SampleOrenNayar(vec3 IncomingTangentSpaceDirection, vec3 OutgoingTangentSpaceDirection, vec3 Albedo, float Sigma)
{
	float Sigma2 = Sigma * Sigma;
	float A = 1.f - (Sigma2 * 0.5f / (Sigma2 + 0.33f));
	float B = 0.45f * Sigma2 / (Sigma2 + 0.09f);
	float Alpha = IncomingTangentSpaceDirection.y > OutgoingTangentSpaceDirection.y ? acos(IncomingTangentSpaceDirection.y) : acos(OutgoingTangentSpaceDirection.y);
	float Beta = IncomingTangentSpaceDirection.y < OutgoingTangentSpaceDirection.y ? acos(IncomingTangentSpaceDirection.y) : acos(OutgoingTangentSpaceDirection.y);
	float CosPhiIncoming = CosPhi(IncomingTangentSpaceDirection);
	float SinPhiIncoming = SinPhi(IncomingTangentSpaceDirection);
	float CosPhiOutgoing = CosPhi(OutgoingTangentSpaceDirection);
	float SinPhiOutgoing = SinPhi(OutgoingTangentSpaceDirection);
	float DeltaCosPhi = CosPhiIncoming * CosPhiOutgoing + SinPhiIncoming * SinPhiOutgoing;
	vec3 Result = M_INV_PI * Albedo * (A + (B * max(0.f, DeltaCosPhi) * sin(Alpha) * tan(Beta)));

	return Result;
}

float PDFOrenNayar(vec3 OutgoingTangentSpaceDirection)
{
	return OutgoingTangentSpaceDirection.y * M_INV_PI;
}

vec4 SampleMaterial(FDeviceMaterial Material, inout FRayData RayData, vec3 NormalInWorldSpace, inout FSamplingState SamplingState, bool bFrontFacing)
{
	float LayerSample = RandomFloat(SamplingState);
	uint RayType = SelectLayer(Material, LayerSample);
	RayData.RayFlags = 0u;
	vec4 BXDF = vec4(1.f);

	switch (RayType)
	{
	case DIFFUSE_LAYER:
	{
		vec3 TangentSpaceReflectionDirection = ScatterOrenNayar(SamplingState);
		mat3 TNBMatrix = CreateTNBMatrix(NormalInWorldSpace);
		vec3 TangentSpaceViewDirection = RayData.Direction.xyz * TNBMatrix;
		BXDF.xyz = SampleOrenNayar(-TangentSpaceViewDirection, TangentSpaceReflectionDirection, Material.BaseColor, Material.DiffuseRoughness * M_PI_2);
		BXDF.w = PDFOrenNayar(TangentSpaceReflectionDirection);
		RayData.Direction.xyz = TangentSpaceReflectionDirection * transpose(TNBMatrix);
		break;
	}
	case SPECULAR_LAYER:
	{
		/// -1.f means that by default ray considered to be  lost in the process of multiple scattering
		BXDF = vec4(Material.SpecularColor, -1.f);
		mat3 TNBMatrix = CreateTNBMatrix(NormalInWorldSpace);
		vec3 TangentSpaceViewDirection = RayData.Direction.xyz * TNBMatrix;

		for (int i = 0; i < 16; ++i)
		{
			vec2 RandomSquare = Sample2DUnitQuad(SamplingState);
			vec3 NewNormal = SampleGGXVNDF(-TangentSpaceViewDirection.xzy, Material.SpecularRoughness * Material.SpecularRoughness, Material.SpecularRoughness * Material.SpecularRoughness, RandomSquare.x, RandomSquare.y).xzy;
			TangentSpaceViewDirection = reflect(TangentSpaceViewDirection, NewNormal);
			if (dot(vec3(0, 1, 0), TangentSpaceViewDirection) > 0.)
			{
				/// Ray successfully left the surface on the correct side
				BXDF.w = 1.f;
				RayData.Direction.xyz = TangentSpaceViewDirection * transpose(TNBMatrix);
				break;
			}
		}
		/// If ray failed to leave the surface, then it's direction is not changed, and thus shouldn't be used later
		break;
	}
	case TRANSMISSION_LAYER:
	{
		BXDF.xyz = Material.TransmissionColor;
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
				BXDF.xyz *= EtaRatio * EtaRatio;
			}
		}

		RayData.Eta = Material.SpecularIOR;
		break;
	}
	case SUBSURFACE_LAYER:
	{
		BXDF.xyz = Material.SubsurfaceColor;
		break;
	}
	case SHEEN_LAYER:
	{
		BXDF.xyz = Material.SheenColor;
		break;
	}
	case COAT_LAYER:
	{
		BXDF.xyz = Material.CoatColor;
		break;
	}
	case EMISSION_LAYER:
	{
		BXDF.xyz = Material.EmissionColor;
		break;
	}
	}

	RayData.RayFlags |= RayType;
	return BXDF;
}
#endif // PROCESS_MATERIAL_INTERACTION_H