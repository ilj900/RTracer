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

/// Cook-Torrance GGX
float DistributionGGX(float NDotH, float Roughness)
{
	float A = Roughness * Roughness;
	float A2 = A * A;
	float NDotH2 = NDotH * NDotH;
	float Nominator = A2;
	float Denominator = (NDotH2 * (A2 - 1.) + 1.);
	Denominator = M_PI * Denominator * Denominator;

	return Nominator / Denominator;
}

float GeometrySchlickGGX(float NDotV, float Roughness)
{
	float R = Roughness + 1.;
	float K = (R * R) / 8.;
	float Nominator = NDotV;
	float Denominator = NDotV * (1. - K) + K;
	return Nominator / Denominator;
}

float GeometrySmith(float NDotV, float NDotL, float Roughness)
{
	float GGX2 = GeometrySchlickGGX(NDotV, Roughness);
	float GGX1 = GeometrySchlickGGX(NDotL, Roughness);
	return GGX1 * GGX2;
}

vec3 FresnelSchlick(float CosTheta, vec3 F0)
{
	return F0 + (1. - F0) * pow(clamp(1.f - CosTheta, 0, 1), 5.0);
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
	return SampleCosineHemisphereMalleys(SamplingState);
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

/// Scatter ray based on material properties.
float ScatterMaterial(FDeviceMaterial Material, out uint RayType, inout FRayData RayData, inout FSamplingState SamplingState, bool bFrontFacing)
{
	float LayerSample = RandomFloat(SamplingState);
	RayType = SelectLayer(Material, LayerSample);
	float PDF = 0.f;

	switch (RayType)
	{
		case DIFFUSE_LAYER:
		{
			ShadingData.TangentSpaceOutgoingDirection = ScatterOrenNayar(SamplingState);
#define OREN_NAYAR
#ifdef OREN_NAYAR
			PDF = PDFOrenNayar(ShadingData.TangentSpaceOutgoingDirection);
#else
			PDF = PDFLambertian(ShadingData.TangentSpaceOutgoingDirection);
#endif
			ShadingData.WorldSpaceOutgoingDirection = ShadingData.TangentSpaceOutgoingDirection * ShadingData.TransposedTNBMatrix;
			ShadingData.IsScatteredRaySingular = false;
			break;
		}
		case SPECULAR_LAYER:
		{
			vec3 TangentSpaceViewDirection = ShadingData.TangentSpaceIncomingDirection;

			if (Material.SpecularRoughness == 0.f)
			{
				TangentSpaceViewDirection = reflect(TangentSpaceViewDirection, vec3(0, 1, 0));

				ShadingData.TangentSpaceOutgoingDirection = TangentSpaceViewDirection;
				ShadingData.WorldSpaceOutgoingDirection = ShadingData.TangentSpaceOutgoingDirection * ShadingData.TransposedTNBMatrix;

				ShadingData.IsScatteredRaySingular = true;
				PDF = 1.f;
			}
			else
			{
				/// try to multi-scatter the ray up to 16 times
				for (int i = 0; i < 16; ++i)
				{
					vec2 RandomSquare = Sample2DUnitQuad(SamplingState);
					/// We invert the TangentSpaceViewDirection because if i == 0 then it's ray's direction that points to (under) the surface, if i != 0, the only way we get here is if ray is still pointing under the surface.
					vec3 NewNormal = SampleGGXVNDF(-TangentSpaceViewDirection.xzy, Material.SpecularRoughness * Material.SpecularRoughness, Material.SpecularRoughness * Material.SpecularRoughness, RandomSquare.x, RandomSquare.y).xzy;
					TangentSpaceViewDirection = reflect(TangentSpaceViewDirection, NewNormal);

					/// If ray's on the right side of the surface, then leave it
					if (dot(vec3(0, 1, 0), TangentSpaceViewDirection) > 0.)
					{
						ShadingData.TangentSpaceOutgoingDirection = TangentSpaceViewDirection;
						ShadingData.WorldSpaceOutgoingDirection = ShadingData.TangentSpaceOutgoingDirection * ShadingData.TransposedTNBMatrix;

						vec3 ApproximatedNormal = normalize((-ShadingData.TangentSpaceIncomingDirection + ShadingData.TangentSpaceOutgoingDirection));
						PDF = VNDPDF(ApproximatedNormal.xzy, Material.SpecularRoughness * Material.SpecularRoughness, Material.SpecularRoughness * Material.SpecularRoughness, -ShadingData.TangentSpaceIncomingDirection.xzy);
						break;
					}
				}

				ShadingData.IsScatteredRaySingular = false;
			}
			/// If ray failed to leave the surface, then it's direction is not changed, and thus shouldn't be used later
			break;
		}
		case TRANSMISSION_LAYER:
		{
			float IOR1 = RayData.Eta;
			float IOR2 = bFrontFacing ? Material.SpecularIOR : 1;
			float EtaRatio = IOR1 / IOR2;

			float R0 = (IOR1 - IOR2) / (IOR1 + IOR2);
			R0 *= R0;

			vec3 TangentSpaceViewDirection = ShadingData.TangentSpaceIncomingDirection;

			if (Material.TransmissionRoughness == 0.f)
			{
				ShadingData.IsScatteredRaySingular = true;
				PDF = 1.f;

				/// NDotI is not equal to cos(theta) cause I in NDotI points towards the surface
				float NDotI = dot(vec3(0, 1, 0), TangentSpaceViewDirection);
                float CosTheta = abs(NDotI);
				float RTheta = R0 + (1. - R0) * pow(1. - CosTheta, 5.f);

				/// Decide on whether the ray is reflected or refracted
				float RF = RandomFloat(SamplingState);

				if (RF < RTheta)
				{
					/// Reflected it be
					RayType = SPECULAR_LAYER;
					ShadingData.TangentSpaceOutgoingDirection = reflect(TangentSpaceViewDirection, vec3(0, 1, 0));
					ShadingData.WorldSpaceOutgoingDirection = ShadingData.TangentSpaceOutgoingDirection * ShadingData.TransposedTNBMatrix;
				}
				else
				{
					/// Refraction or TIR is happening
					float k = 1. - (EtaRatio * EtaRatio * (1. - (NDotI * NDotI)));

					if (k < 0.)
					{
						RayType = SPECULAR_LAYER;

						/// Total internal reflection is happening
						ShadingData.TangentSpaceOutgoingDirection = reflect(TangentSpaceViewDirection, vec3(0, 1, 0));
						ShadingData.WorldSpaceOutgoingDirection = ShadingData.TangentSpaceOutgoingDirection * ShadingData.TransposedTNBMatrix;
					}
					else
					{
						/// Refraction
						vec3 RefractedDirection = normalize(EtaRatio * TangentSpaceViewDirection - (EtaRatio * NDotI + sqrt(k)) * vec3(0, 1, 0));
						ShadingData.TangentSpaceOutgoingDirection = RefractedDirection;
						ShadingData.WorldSpaceOutgoingDirection = ShadingData.TangentSpaceOutgoingDirection * ShadingData.TransposedTNBMatrix;

						/// Also, ray is now traveling in a new medium
                        RayData.Eta = IOR2;
					}
				}
			}
			else
			{
                ShadingData.IsScatteredRaySingular = false;

				for (int i = 0; i < 16; ++i)
				{
					vec2 RandomSquare = Sample2DUnitQuad(SamplingState);
					vec3 NewNormal = SampleGGXVNDF(-TangentSpaceViewDirection.xzy, Material.TransmissionRoughness * Material.TransmissionRoughness, Material.TransmissionRoughness * Material.TransmissionRoughness, RandomSquare.x, RandomSquare.y).xzy;

                    /// NDotI is not equal to cos(theta) cause I in NDotI points towards the surface
                    float NDotI = dot(vec3(0, 1, 0), TangentSpaceViewDirection);
                    float CosTheta = abs(NDotI);
					float RTheta = R0 + (1. - R0) * pow(1. - CosTheta, 5.f);

					/// Decide on whether the ray is reflected or refracted
					float RF = RandomFloat(SamplingState);

					if (RF < RTheta)
					{
						/// Reflected it be
						TangentSpaceViewDirection = reflect(TangentSpaceViewDirection, NewNormal);

						/// If reflected ray's on the correct side, then it's done.
						if (dot(vec3(0, 1, 0), TangentSpaceViewDirection) > 0.)
						{
							RayType = SPECULAR_LAYER;
							ShadingData.TangentSpaceOutgoingDirection = TangentSpaceViewDirection;
							ShadingData.WorldSpaceOutgoingDirection = ShadingData.TangentSpaceOutgoingDirection * ShadingData.TransposedTNBMatrix;

							vec3 ApproximatedNormal = normalize((-ShadingData.TangentSpaceIncomingDirection + ShadingData.TangentSpaceOutgoingDirection));
							PDF = VNDPDF(ApproximatedNormal.xzy, Material.TransmissionRoughness * Material.TransmissionRoughness, Material.TransmissionRoughness * Material.TransmissionRoughness, -ShadingData.TangentSpaceIncomingDirection.xzy);
							break;
						}
					}
					else
					{
						/// Refraction or TIR is happening
						float k = 1. - (EtaRatio * EtaRatio * (1. - (NDotI * NDotI)));

						if (k < 0.)
						{
							/// Total internal reflection is happening
							TangentSpaceViewDirection = reflect(TangentSpaceViewDirection, NewNormal);

							/// If reflected ray's on the correct side, then it's done.
							if (dot(vec3(0, 1, 0), TangentSpaceViewDirection) > 0.)
							{
								RayType = SPECULAR_LAYER;
								ShadingData.TangentSpaceOutgoingDirection = TangentSpaceViewDirection;
								ShadingData.WorldSpaceOutgoingDirection = ShadingData.TangentSpaceOutgoingDirection * ShadingData.TransposedTNBMatrix;

								vec3 ApproximatedNormal = normalize((-ShadingData.TangentSpaceIncomingDirection + ShadingData.TangentSpaceOutgoingDirection));
								PDF = VNDPDF(ApproximatedNormal.xzy, Material.TransmissionRoughness * Material.TransmissionRoughness, Material.TransmissionRoughness * Material.TransmissionRoughness, -ShadingData.TangentSpaceIncomingDirection.xzy);
								break;
							}
						}
						else
						{
							/// Refraction
							vec3 RefractedDirection = normalize(EtaRatio * TangentSpaceViewDirection - (EtaRatio * NDotI + sqrt(k)) * NewNormal);

							/// Find a normal that should be used to refract the ray this way
							float CosThetaI = dot(-ShadingData.TangentSpaceIncomingDirection, RefractedDirection);
							k = 1.f - EtaRatio * EtaRatio * (1.f - CosThetaI * CosThetaI);
							vec3 ApproximatedNormal = normalize((ShadingData.TangentSpaceIncomingDirection * EtaRatio - RefractedDirection) / sqrt(k));

							/// Get PDF
							PDF = VNDPDF(ApproximatedNormal.xzy, Material.TransmissionRoughness * Material.TransmissionRoughness, Material.TransmissionRoughness * Material.TransmissionRoughness, -ShadingData.TangentSpaceIncomingDirection.xzy);
							PDF /= EtaRatio * EtaRatio;
							/// Also, ray is now traveling in a new media
                            RayData.Eta = Material.SpecularIOR;

							ShadingData.TangentSpaceOutgoingDirection = RefractedDirection;
							ShadingData.WorldSpaceOutgoingDirection = ShadingData.TangentSpaceOutgoingDirection * ShadingData.TransposedTNBMatrix;

							break;
						}
					}
				}
			}

			/// If we are here, it means that ray's failed to leave the surface, thus PDF remains 0
			break;
		}
		case SUBSURFACE_LAYER:
		{
			PDF = 1.f;
			break;
		}
		case SHEEN_LAYER:
		{
			PDF = 1.f;
			break;
		}
		case COAT_LAYER:
		{
			PDF = 1.f;
			break;
		}
		case EMISSION_LAYER:
		{
			PDF = 1.f;
			break;
		}
	}

	return PDF;
}

/// RayData here already scattered, so, it contains an outgoing ray direction
/// Incoming is the direction from the light to the shading point
/// Outgoing is the direction from the shading point to the camera
vec3 EvaluateMaterialInteraction(FDeviceMaterial Material, uint RayType)
{
	vec3 BXDF = vec3(0.f);

	switch (RayType)
	{
		case DIFFUSE_LAYER:
		{
#define OREN_NAYAR
#ifdef OREN_NAYAR
			BXDF = SampleOrenNayar(-ShadingData.TangentSpaceIncomingDirection, ShadingData.TangentSpaceOutgoingDirection, Material.BaseColor, Material.DiffuseRoughness);
#else
			BXDF = SampleLambertian(Material.BaseColor);
#endif
			break;
		}
		case SPECULAR_LAYER:
		{
			if (Material.SpecularRoughness != 0)
			{
				vec3 V = ShadingData.WorldSpaceOutgoingDirection;
				vec3 L = - ShadingData.WorldSpaceIncomingDirection;
				vec3 N = ShadingData.NormalInWorldSpace;
				vec3 H = normalize(L + V);
				vec3 F0 = mix(vec3(0.04), Material.BaseColor, Material.Metalness);

				float NDotL = clamp(dot(N, L), 0, 1);
				float NDotV = clamp(dot(N, V), 0, 1);
				float NDotH = clamp(dot(N, H), 0, 1);
				float LDotH = clamp(dot(L, H), 0, 1);

				float D = DistributionGGX(NDotH, Material.SpecularRoughness);
				float G = GeometrySmith(NDotV, NDotL, Material.SpecularRoughness);
				vec3 F = FresnelSchlick(LDotH, F0);

				BXDF = D * G * F / (4 * max (NDotV * NDotL, 0.001));

				/// From https://learnopengl.com/PBR/Lighting
				/// We still has to calculate diffuse BRDF...
				vec3 SpecularRatio = F;
				vec3 DiffuseRatio = vec3(1.f) - SpecularRatio;
				DiffuseRatio *= 1.f - Material.Metalness;
				vec3 DiffuseBRDF = vec3(0);

#ifdef OREN_NAYAR
				DiffuseBRDF = SampleOrenNayar(-ShadingData.TangentSpaceIncomingDirection, ShadingData.TangentSpaceOutgoingDirection, Material.BaseColor, Material.DiffuseRoughness);
#else
				DiffuseBRDF = SampleLambertian(Material.BaseColor);
#endif

				BXDF *= Material.SpecularColor;
				BXDF += DiffuseBRDF * DiffuseRatio;
			}
			else
			{
				BXDF = Material.SpecularColor;
			}
			break;
		}
		case TRANSMISSION_LAYER:
		{
			BXDF = Material.TransmissionColor;
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

	return BXDF;
}

/// It's like EvaluateMaterialInteraction, but it takes direction to the light source, not the scattered direction
vec3 EvaluateMaterialInteractionFromDirection(FDeviceMaterial Material, uint RayType, vec3 WorldSpaceLightDirection)
{
	vec3 BXDF = vec3(0.f);

	switch (RayType)
	{
		case DIFFUSE_LAYER:
		{
#define OREN_NAYAR
#ifdef OREN_NAYAR
			vec3 TangentSpaceLightDirectio = WorldSpaceLightDirection * ShadingData.TNBMatrix;
			BXDF = SampleOrenNayar(-ShadingData.TangentSpaceIncomingDirection, TangentSpaceLightDirectio, Material.BaseColor, Material.DiffuseRoughness);
#else
			BXDF = SampleLambertian(Material.BaseColor);
#endif
			break;
		}
		case SPECULAR_LAYER:
		{
			if (Material.SpecularRoughness != 0)
			{
				vec3 V = WorldSpaceLightDirection;
				vec3 L = - ShadingData.WorldSpaceIncomingDirection;
				vec3 N = ShadingData.NormalInWorldSpace;
				vec3 H = normalize(L + V);
				vec3 F0 = mix(vec3(0.04), Material.BaseColor, Material.Metalness);

				float NDotL = clamp(dot(N, L), 0, 1);
				float NDotV = clamp(dot(N, V), 0, 1);
				float NDotH = clamp(dot(N, H), 0, 1);
				float LDotH = clamp(dot(L, H), 0, 1);

				float D = DistributionGGX(NDotH, Material.SpecularRoughness);
				float G = GeometrySmith(NDotV, NDotL, Material.SpecularRoughness);
				vec3 F = FresnelSchlick(LDotH, F0);

				BXDF = D * G * F / (4 * max (NDotV * NDotL, 0.001));

				/// From https://learnopengl.com/PBR/Lighting
				/// We still has to calculate diffuse BRDF...
				vec3 SpecularRatio = F;
				vec3 DiffuseRatio = vec3(1.f) - SpecularRatio;
				DiffuseRatio *= 1.f - Material.Metalness;
				vec3 DiffuseBRDF = vec3(0);

#ifdef OREN_NAYAR
				vec3 TangentSpaceLightDirectio = WorldSpaceLightDirection * ShadingData.TNBMatrix;
				DiffuseBRDF = SampleOrenNayar(-ShadingData.TangentSpaceIncomingDirection, TangentSpaceLightDirectio, Material.BaseColor, Material.DiffuseRoughness);
#else
				DiffuseBRDF = SampleLambertian(Material.BaseColor);
#endif

				BXDF *= Material.SpecularColor;
				BXDF += DiffuseBRDF * DiffuseRatio;
			}
			else
			{
				BXDF = Material.SpecularColor;
			}
			break;
		}
		case TRANSMISSION_LAYER:
		{
			BXDF = Material.TransmissionColor;
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

	return BXDF;
}

/// Evaluate PDF of ray coming from that direction
float EvaluateScatteringPDF(FDeviceMaterial Material, uint RayType, vec3 WorldSpaceLightDirection)
{
	vec3 TangentSpaceLightDirection = WorldSpaceLightDirection * ShadingData.TNBMatrix;

	switch (RayType)
	{
		case DIFFUSE_LAYER:
		{
#define OREN_NAYAR
#ifdef OREN_NAYAR
			return PDFOrenNayar(TangentSpaceLightDirection);
#else
			return PDFLambertian(TangentSpaceLightDirection);
#endif
		}
		case SPECULAR_LAYER:
		{
			if (Material.SpecularRoughness == 0.f)
			{
				if (ShadingData.WorldSpaceOutgoingDirection == WorldSpaceLightDirection)
				{
					return 1.f;
				}
				else
				{
					return 0.f;
				}
			}
			else
			{
				vec3 ApproximatedNormal = normalize((-ShadingData.TangentSpaceIncomingDirection + TangentSpaceLightDirection));
				return VNDPDF(ApproximatedNormal, Material.SpecularRoughness * Material.SpecularRoughness, Material.SpecularRoughness * Material.SpecularRoughness, -ShadingData.TangentSpaceIncomingDirection);
			}
		}
		case TRANSMISSION_LAYER:
		{
			/// If the ray is over the surface, then in case of transmissive it cannot be refracted.
			return 0.f;
		}
		case SUBSURFACE_LAYER:
		{
			return 1.f;
		}
		case SHEEN_LAYER:
		{
			return 1.f;
		}
		case COAT_LAYER:
		{
			return 1.f;
		}
		case EMISSION_LAYER:
		{
			return 1.f;
		}
	}

	return 0.f;
}

#endif // PROCESS_MATERIAL_INTERACTION_H