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

vec3 SampleLambertian(vec3 Albedo)
{
	return Albedo * M_INV_PI;
}

float PDFLambertian(vec3 OutgoingTangentSpaceDirection)
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
#define OREN_NAYAR
#ifdef OREN_NAYAR
		BXDF.xyz = SampleOrenNayar(-TangentSpaceViewDirection, TangentSpaceReflectionDirection, Material.BaseColor, Material.DiffuseRoughness);
		BXDF.w = PDFLambertian(TangentSpaceReflectionDirection);
#else
		BXDF.xyz = SampleLambertian(Material.BaseColor);
		BXDF.w = PDFOrenNayar(TangentSpaceReflectionDirection);
#endif
		RayData.Direction.xyz = TangentSpaceReflectionDirection * transpose(TNBMatrix);
		break;
	}
	case SPECULAR_LAYER:
	{
		/// We assume that by default that our attempt to scatter the ray fails, thus PDF would be 0.f
		BXDF = vec4(Material.SpecularColor, 0.f);

		/// Now lets try to scatter the ray
		mat3 TNBMatrix = CreateTNBMatrix(NormalInWorldSpace);
		vec3 TangentSpaceViewDirection = RayData.Direction.xyz * TNBMatrix;

		/// Multi-scatter 16 times
		for (int i = 0; i < 16; ++i)
		{
			vec2 RandomSquare = Sample2DUnitQuad(SamplingState);
			/// We invert the TangentSpaceViewDirection because if i == 0 then it's ray's direction that points to (under) the surface, it i != 0, the only way we get here is if ray is still pointing under the surface.
			vec3 NewNormal = SampleGGXVNDF(-TangentSpaceViewDirection.xzy, Material.SpecularRoughness * Material.SpecularRoughness, Material.SpecularRoughness * Material.SpecularRoughness, RandomSquare.x, RandomSquare.y).xzy;
			TangentSpaceViewDirection = reflect(TangentSpaceViewDirection, NewNormal);

			/// If ray's on the right side of the surface, then leave it
			if (dot(vec3(0, 1, 0), TangentSpaceViewDirection) > 0.)
			{
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
		BXDF = vec4(Material.TransmissionColor, 0.f);

		mat3 TNBMatrix = CreateTNBMatrix(NormalInWorldSpace);
		vec3 TangentSpaceViewDirection = RayData.Direction.xyz * TNBMatrix;

		float IOR1 = bFrontFacing ? RayData.Eta : Material.SpecularIOR;
		float IOR2 = bFrontFacing ? Material.SpecularIOR : 1;
		float EtaRatio = IOR1 / IOR2;

		float R0 = (IOR1 - IOR2) / (IOR1 + IOR2);
		R0 *= R0;

		for (int i = 0; i < 16; ++i)
		{
			vec2 RandomSquare = Sample2DUnitQuad(SamplingState);
			/// We invert the TangentSpaceViewDirection because if i == 0 then it's ray's direction that points to (under) the surface, it i != 0, the only way we get here is if ray is still pointing under the surface.
			vec3 NewNormal = SampleGGXVNDF(-TangentSpaceViewDirection.xzy, Material.TransmissionRoughness * Material.TransmissionRoughness, Material.TransmissionRoughness * Material.TransmissionRoughness, RandomSquare.x, RandomSquare.y).xzy;

			/// NDotI also equals to cos(angle)
			/// Ray's direction is inverted cause normal is guaranteed to be visible.
			float NDotI = dot(NewNormal, -TangentSpaceViewDirection);

			float RTheta = R0 + (1. - R0) * pow(1. - abs(NDotI), 5.f);

			/// Decide on whether the ray is reflected or refracted
			float RF = RandomFloat(SamplingState);

			if (RF < RTheta)
			{
				TangentSpaceViewDirection = reflect(TangentSpaceViewDirection, NewNormal);

				/// If reflected ray's on the correct side, then it's done.
				if (dot(vec3(0, 1, 0), TangentSpaceViewDirection) > 0.)
				{
					RayType = SPECULAR_LAYER;
					BXDF.w = 1.f;
					RayData.Direction.xyz = TangentSpaceViewDirection * transpose(TNBMatrix);
					break;
				}
			}
			else
			{
				/// Refraction or TIR is happening
				float k = 1. - EtaRatio * EtaRatio * (1. - NDotI * NDotI);

				if (k < 0.)
				{
					/// Total internal reflection is happening
					TangentSpaceViewDirection = reflect(TangentSpaceViewDirection, NewNormal);

					/// If reflected ray's on the correct side, then it's done.
					if (dot(vec3(0, 1, 0), TangentSpaceViewDirection) > 0.)
					{
						RayType = SPECULAR_LAYER;
						BXDF.w = 1.f;
						RayData.Direction.xyz = TangentSpaceViewDirection * transpose(TNBMatrix);
						break;
					}
				}
				else
				{
					/// Refraction
					TangentSpaceViewDirection = EtaRatio * TangentSpaceViewDirection - (EtaRatio * NDotI + sqrt(k)) * NewNormal;
					BXDF.xyz *= EtaRatio * EtaRatio;
					BXDF.w = 1.f;
					/// Also, ray is now traveling in a new media
					RayData.Eta = Material.SpecularIOR;
					RayData.Direction.xyz = TangentSpaceViewDirection * transpose(TNBMatrix);
					break;
				}
			}
		}

		/// If we are here, it means that ray's failed to leave the surface, thus PDF remains 0
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