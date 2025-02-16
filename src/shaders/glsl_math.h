#ifndef GLSL_MATH_H
#define GLSL_MATH_H

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
	T = normalize(cross(Normal, B));
	return mat3(T, Normal, B);
}

#endif //GLSL_MATH_H