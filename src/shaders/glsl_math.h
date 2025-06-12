#ifndef GLSL_MATH_H
#define GLSL_MATH_H

#include "common_defines.h"

vec3 UnitSphericalToCartesian(float Phi, float Theta)
{
	vec3 Result;

	Result.x = cos(Theta) * sin(Phi);
	Result.y = cos(Phi);
	Result.z = sin(Theta) * sin(Phi);

	return Result;
}

mat3 CreateTNBMatrix(vec3 Normal)
{
	vec3 T = Normal.x == 0. ? vec3(1, 0, 0) : vec3(0, 1, 0);
	vec3 B = normalize(cross(Normal, T));
	T = cross(B, Normal);
	return mat3(T, Normal, B);
}

float SinPhi(vec3 Vector)
{
	float Cos2Theta = Vector.y * Vector.y;
	float SinTheta = sqrt(max(0.f, 1.f - Cos2Theta));
	return SinTheta == 0.f ? 0.f : clamp(Vector.z / SinTheta, -1.f, 1.f);
}

float CosPhi(vec3 Vector)
{
	float Cos2Theta = Vector.y * Vector.y;
	float SinTheta = sqrt(max(0.f, 1.f - Cos2Theta));
	return SinTheta == 0.f ? 1.f : clamp(Vector.x / SinTheta, -1.f, 1.f);
}

vec2 Vec3ToSphericalUV(vec3 Direction, float PhiRotation)
{
	float Phi = atan(Direction.z, Direction.x);
	Phi += PhiRotation;
	Phi = mod(Phi, M_2_PI);
	float Theta = acos(Direction.y);
	Phi /= M_2_PI;
	Theta /= M_PI;

	return vec2(Phi, Theta);
}

#endif //GLSL_MATH_H