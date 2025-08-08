#ifndef BXDF_H
#define BXDF_H

/// Trowbridge–Reitz GGX
/// Calculates D (Distribution) term for BRDF
/// incoming Roughness is expected to be [0, 1] (The default material's roughness)
float DistributionGGX(float NDotH, float Roughness)
{
	float A = Roughness * Roughness;
	float A2 = A * A;
	float NDotH2 = NDotH * NDotH;
	float Numerator = A2;
	float Denominator = NDotH2 * (A2 - 1.) + 1.;
	Denominator = M_PI * Denominator * Denominator;

	Denominator = max(Denominator, 1e-6);
	return Numerator / Denominator;
}

/// Schlick-GGX approximation for geometric attenuation (shadowing/masking) using roughness.
/// NDotV is not necessary angle between normal and view direction. It's suitable for light direction
float GeometrySchlickGGX(float NDotV, float Roughness)
{
	float R = Roughness + 1.;
	float K = (R * R) / 8.;
	float Numerator = NDotV;
	float Denominator = NDotV * (1. - K) + K;
	return Numerator / Denominator;
}

/// Smith's GGX
/// Calculates G (Geometric) term for BRDF
float GeometrySmith(float NDotV, float NDotL, float Roughness)
{
	float GGX2 = GeometrySchlickGGX(NDotV, Roughness);
	float GGX1 = GeometrySchlickGGX(NDotL, Roughness);
	return GGX1 * GGX2;
}

/// Fresnel Schlick's approximation
/// Calculates F (Fresnel) term for BRDF
/// Defines the part of light that is reflected
/// Other part is "passed inside" the material
vec3 FresnelSchlick(float CosTheta, vec3 F0)
{
	return F0 + (1. - F0) * pow(clamp(1.f - CosTheta, 0, 1), 5.0);
}

/// Random visible normal generator
/// U1 and U2 - two random numbers [0, 1]
/// AlphaX and AlphaZ - roughness [0, 1]
/// The coordinates basis of this code is X - right, Y - front and Z - up.
/// https://jcgt.org/published/0007/04/01/paper.pdf
FVector3 SampleGGXVNDF(FVector3 ViewDirection, float AlphaX, float AlphaZ, float U1, float U2)
{
//#define MINE
#ifdef MINE
	/// Transform the view direction to the hemisphere configuration
	FVector3 ViewDirectionHemisphere = normalize(FVector3(AlphaX * ViewDirection.x, ViewDirection.y, AlphaZ * ViewDirection.z));
	/// Build an orthonormal basis
	float LengthSquared = ViewDirectionHemisphere.x * ViewDirectionHemisphere.x + ViewDirectionHemisphere.z * ViewDirectionHemisphere.z;
	FVector3 Tangent1 = LengthSquared > 0 ? FVector3(-ViewDirectionHemisphere.z, 0, ViewDirectionHemisphere.x) / sqrt(LengthSquared) : FVector3(1, 0, 0);
	FVector3 Tangent2 = cross(Tangent1, ViewDirectionHemisphere);
	/// Parametrization of the projected area
	float R = sqrt(U1);
	float Phi = M_2_PI * U2;
	float T1 = R * cos(Phi);
	float T2 = R * sin(Phi);
	float S = 0.5 * (1. + ViewDirectionHemisphere.y);
	T2 = (1. - S) * sqrt(1. - T1 * T1) + S * T2;
	/// Re-projection onto a hemisphere
	FVector3 NewNormalHemisphere = Tangent1 * T1 + Tangent2 * T2 + ViewDirectionHemisphere * sqrt(max(0, 1. - T1 * T1 - T2 * T2));
	/// Transforming the normal back to the ellipsoid configuration
	FVector3 NewNormal = normalize(FVector3(AlphaX * NewNormalHemisphere.x, max(0.0, NewNormalHemisphere.y), AlphaZ * NewNormalHemisphere.z));
	return NewNormal;

#else
	// Section 3.2: transforming the view direction to the hemisphere configuration
	FVector3 Vh = normalize(FVector3(AlphaX * ViewDirection.x, AlphaZ * ViewDirection.y, ViewDirection.z));
	// Section 4.1: orthonormal basis (with special case if cross product is zero)
	float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
	FVector3 T1 = lensq > 0 ? normalize(FVector3(-Vh.y, Vh.x, 0)) : FVector3(1,0,0);
	FVector3 T2 = cross(Vh, T1);
	// Section 4.2: parameterization of the projected area
	float r = sqrt(U1);
	float phi = 2.0 * M_PI * U2;
	float t1 = r * cos(phi);
	float t2 = r * sin(phi);
	float s = 0.5 * (1.0 + Vh.z);
	t2 = (1.0 - s)*sqrt(1.0 - t1*t1) + s*t2;
	// Section 4.3: reprojection onto hemisphere
	FVector3 Nh = T1 * t1 + T2 * t2 + Vh * sqrt(max(0.0, 1.0 - t1*t1 - t2*t2));
	// Section 3.4: transforming the normal back to the ellipsoid configuration
	FVector3 Ne = normalize(FVector3(AlphaX * Nh.x, AlphaZ * Nh.y, max(0.0, Nh.z)));
	return Ne;
#endif
}

/// Computes pdf of a sampled Normal
/// Also pay close attention to axis: X - right, Y - front and Z - up.
/// https://jcgt.org/published/0007/04/01/paper.pdf
float VNDPDF(FVector3 Normal, float AlphaX, float AlphaY, FVector3 ViewDirection)
{
	float VarA = Normal.x * Normal.x / (AlphaX * AlphaX) + Normal.y * Normal.y / (AlphaY * AlphaY) + Normal.z * Normal.z;
	float DN = 1.f / (M_PI * AlphaX * AlphaY * VarA * VarA);
	float VarB = 1.f + (AlphaX * AlphaX * ViewDirection.x * ViewDirection.x + AlphaY * AlphaY * ViewDirection.y * ViewDirection.y) / (ViewDirection.z * ViewDirection.z);
	float G1V = 1.f / (1.f + 0.5f * (-1 + sqrt(VarB)));
	float DVN = G1V * clamp(dot(ViewDirection, Normal), 0, 1) * DN / dot(ViewDirection, FVector3(0, 0, 1));
	float PDF = 0.25f * DVN / dot(ViewDirection, Normal);
	return PDF;
}
#endif // BXDF_H