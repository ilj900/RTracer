#ifndef BXDF_H
#define BXDF_H

FVector2 CDFCookTorrance(float a2, float e1, float e2)
{
	float Theta = acos(sqrt((1 - e1) / (e1 * (a2 - 1) + 1)));
	float Phi = M_2_PI * e2;
	return FVector2(Theta, Phi);
}

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
	FVector3 T1 = lensq > 0 ? FVector3(-Vh.y, Vh.x, 0) / sqrt(lensq) : FVector3(1,0,0);
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
	FVector3 Ne = normalize(FVector3(AlphaX * Nh.x, AlphaZ * Nh.y, std::max<float>(0.0, Nh.z)));
	return Ne;
#endif
}
#endif // BXDF_H