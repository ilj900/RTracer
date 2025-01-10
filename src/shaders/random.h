#ifndef CMJ_H
#define CMJ_H

#define CMJ_GRID_LINEAR_SIZE 64
#define CMJ_TOTAL_GRID_SIZE (CMJ_GRID_LINEAR_SIZE * CMJ_GRID_LINEAR_SIZE)

#ifndef __cplusplus
#define FVector4 vec4
#define FVector3 vec3
#define FVector2 vec2
#define FMatrix4 mat4
#define uint32_t uint
#else
#define inout
#endif

#define SAMPLE_TYPE_GENERATE_RAYS 	0x10000001
#define SAMPLE_TYPE_INTERACT_RAYS 	0x20000001
#define SAMPLE_TYPE_LIGHT		 	0x30000001

struct FSamplingState
{
	uint32_t RenderIteration;
	uint32_t Bounce;
	uint32_t Generation;
	uint32_t PixelIndex;
	uint32_t Type;
};

uint32_t Permute(uint32_t i, uint32_t l, uint32_t p)
{
	uint32_t w = l - 1;
	w |= w >> 1;
	w |= w >> 2;
	w |= w >> 4;
	w |= w >> 8;
	w |= w >> 16;

	do
	{
		i ^= p;
		i *= 0xe170893d;
		i ^= p >> 16;
		i ^= (i & w) >> 4;
		i ^= p >> 8;
		i *= 0x0929eb3f;
		i ^= p >> 23;
		i ^= (i & w) >> 1;
		i *= 1 | p >> 27;
		i *= 0x6935fa69;
		i ^= (i & w) >> 11;
		i *= 0x74dcb303;
		i ^= (i & w) >> 2;
		i *= 0x9e501cc3;
		i ^= (i & w) >> 2;
		i *= 0xc860a3df;
		i &= w;
		i ^= i >> 5;
	} while (i >= l);

	return (i + p) % l;
}

float RandomFloat(uint32_t i, uint32_t p)
{
	i ^= p;
	i ^= i >> 17;
	i ^= i >> 10;
	i *= 0xb36534e5;
	i ^= i >> 12;
	i ^= i >> 21;
	i *= 0x93fc4795;
	i ^= 0xdf6e307f;
	i ^= i >> 17;
	i *= 1 | p >> 18;
	return i * (1.0f / 4294967808.0f);
}

FVector2 CMJ(uint32_t s, uint32_t m, uint32_t n, uint32_t p)
{
	uint32_t sx = Permute(s % m, m, p * 0xa511e9b3);
	uint32_t sy = Permute(s / m, n, p * 0x63d83595);
	float jx = RandomFloat(s, p * 0xa399d265);
	float jy = RandomFloat(s, p * 0x711ad6a5);

	FVector2 Result =  {(s % m + (sy + jx) / n) / m, ( s / m + (sx + jy) / m) / n};

	return Result;
}

FVector2 Sample2DUnitQuad(inout FSamplingState SamplingState)
{;
	uint32_t StratumIndex = Permute((SamplingState.PixelIndex * 524287 + SamplingState.RenderIteration) % CMJ_TOTAL_GRID_SIZE, CMJ_TOTAL_GRID_SIZE, (SamplingState.PixelIndex * 524287 + SamplingState.RenderIteration) / CMJ_TOTAL_GRID_SIZE);
	uint32_t PermutationIndex = SamplingState.Type + SamplingState.RenderIteration * 256 + SamplingState.Bounce * 16 + SamplingState.Generation;
	SamplingState.Generation += 1;
	return CMJ(StratumIndex, CMJ_GRID_LINEAR_SIZE, CMJ_GRID_LINEAR_SIZE, PermutationIndex);
}

/// Disk centered at x = 0.5 and y = 0.5 with a radius of 0.5
FVector2 Sample2DUnitDisk(inout FSamplingState SamplingState)
{
	FVector2 Sample = Sample2DUnitQuad(SamplingState);
	float Theta = Sample.x * M_2_PI;
	float R = sqrt(Sample.y);
	Sample.x = (R * cos(Theta) + 1) * 0.5;
	Sample.y = (R * sin(Theta) + 1) * 0.5;
	return Sample;
}

FVector3 Sample3DUnitSphere(inout FSamplingState SamplingState)
{
	FVector2 Sample = Sample2DUnitQuad(SamplingState);
	float Theta = Sample.x * M_2_PI;
	float Z = Sample.y * 2 - 1;
	float Z2 = sqrt(1 - (Z * Z));
	FVector3 Result = {1, 0, 0};
	Result.x = Z2 * cos(Theta);
	Result.y = Z2 * sin(Theta);
	Result.z = Z;
	return Result;
}

/// Random vector on a unit hemisphere pointing up (y)
FVector3 Sample3DUnitHemisphere(inout FSamplingState SamplingState)
{
	FVector2 Sample = Sample2DUnitQuad(SamplingState);
	float Theta = Sample.x * M_2_PI;
	float Z = Sample.y * 2 - 1;
	float Z2 = sqrt(1 - (Z * Z));
	FVector3 Result = {1, 0, 0};
	Result.x = Z2 * cos(Theta);
	Result.y = abs(Z2 * sin(Theta));
	Result.z = Z;
	return Result;
}

float RandomFloat(inout FSamplingState SamplingState)
{
	uint32_t StratumIndex = Permute((SamplingState.PixelIndex * 524287 + SamplingState.RenderIteration) % CMJ_TOTAL_GRID_SIZE, CMJ_TOTAL_GRID_SIZE, (SamplingState.PixelIndex * 524287 + SamplingState.RenderIteration) / CMJ_TOTAL_GRID_SIZE);
	uint32_t PermutationIndex = SamplingState.Type + SamplingState.RenderIteration * 256 + SamplingState.Bounce * 16 + SamplingState.Generation;
	SamplingState.Generation += 1;
	return RandomFloat(StratumIndex, PermutationIndex);
}

#endif // CMJ_H