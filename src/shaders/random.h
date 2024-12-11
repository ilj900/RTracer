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

struct FSamplingState
{
	uint32_t Seed;
	uint32_t Bounce;
	uint32_t SampleIndex;
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
	int sx = Permute(s % m, m, p * 0xa511e9b3);
	int sy = Permute(s / m, n, p * 0x63d83595);
	float jx = RandomFloat(s, p * 0xa399d265);
	float jy = RandomFloat(s, p * 0x711ad6a5);

	FVector2 Result =  {(s % m + (sy + jx) / n) / m, ( s / m + (sx + jy) / m) / n};

	return Result;
}

FVector2 Sample2DUnitQuad(FSamplingState SamplingState)
{
	uint32_t Hash = 227 + SamplingState.Seed * 1489 + SamplingState.Bounce * 1399 + SamplingState.SampleIndex * 401;
	return CMJ(Hash % CMJ_TOTAL_GRID_SIZE, CMJ_GRID_LINEAR_SIZE, CMJ_GRID_LINEAR_SIZE, Hash);
}

/// SDisk centered at x = 0.5 and y = 0.5 with a radius of 0.5
FVector2 Sample2DUnitDisk(FSamplingState SamplingState)
{
	uint32_t Hash = 227 + SamplingState.Seed * 1489 + SamplingState.Bounce * 1399 + SamplingState.SampleIndex * 401;
	FVector2 Sample = CMJ(Hash % CMJ_TOTAL_GRID_SIZE, CMJ_GRID_LINEAR_SIZE, CMJ_GRID_LINEAR_SIZE, Hash);
	float Theta = Sample.X * M_2_PI;
	float R = sqrt(Sample.Y);
	Sample.X = (R * cos(Theta) + 1) * 0.5;
	Sample.Y = (R * sin(Theta) + 1) * 0.5;
	return Sample;
}

float RandomFloat(inout FSamplingState SamplingState)
{
	uint32_t Hash = 227 + SamplingState.Seed * 1489 + SamplingState.Bounce * 1399 + SamplingState.SampleIndex * 401;
	SamplingState.SampleIndex += 1;
	return RandomFloat(0, Hash);
}

#endif // CMJ_H