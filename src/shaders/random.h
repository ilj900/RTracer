#ifndef CMJ_H
#define CMJ_H

#define CMJ_GRID_LINEAR_SIZE 4
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

uint32_t Permute(uint32_t I, uint32_t L, uint32_t P)
{
	uint32_t W = L - 1;
	W |= W >> 1;
	W |= W >> 2;
	W |= W >> 4;
	W |= W >> 8;
	W |= W >> 16;

	do
	{
		I ^= P;
		I *= 0xe170893d;
		I ^= P >> 16;
		I ^= (I & W) >> 4;
		I ^= P >> 8;
		I *= 0x0929eb3f;
		I ^= P >> 23;
		I ^= (I & W) >> 1;
		I *= 1 | P >> 27;
		I *= 0x6935fa69;
		I ^= (I & W) >> 11;
		I *= 0x74dcb303;
		I ^= (I & W) >> 2;
		I *= 0x9e501cc3;
		I ^= (I & W) >> 2;
		I *= 0xc860a3df;
		I &= W;
		I ^= I >> 5;
	} while (I >= L);

	return (I + P) % L;
}

float RandomFloat(uint32_t I, uint32_t P)
{
	I ^= P;
	I ^= I >> 17;
	I ^= I >> 10;
	I *= 0xb36534e5;
	I ^= I >> 12;
	I ^= I >> 21;
	I *= 0x93fc4795;
	I ^= 0xdf6e307f;
	I ^= I >> 17;
	I *= 1 | P >> 18;
	return I * (1.f / 4294967808.f);
}

FVector2 CMJ(uint32_t S, uint32_t M, uint32_t N, uint32_t P)
{
	uint32_t SS = Permute(S, N * N, P * 0x91ca3645);
	uint32_t SX = Permute(SS % M, M, P * 0xa511e9b3);
	uint32_t SY = Permute(SS / M, N, P * 0x63d83595);
	float JX = RandomFloat(SS, P * 0xa399d265);
	float JY = RandomFloat(SS, P * 0x711ad6a5);

	FVector2 Result = {(SS % M + (SY + JX) / N) / M, (SS / M + (SX + JY) /M) / N};

	return Result;
}

FVector2 Sample2DUnitQuad(FSamplingState SamplingState)
{
	uint32_t Hash = 227 + SamplingState.Seed * 1489 + SamplingState.Bounce * 1399 + SamplingState.SampleIndex * 401;
	return CMJ(Hash % CMJ_TOTAL_GRID_SIZE, CMJ_GRID_LINEAR_SIZE, CMJ_GRID_LINEAR_SIZE, Hash);
}

FVector2 Sample2DUnitDisk(FSamplingState SamplingState)
{
	uint32_t Hash = 227 + SamplingState.Seed * 1489 + SamplingState.Bounce * 1399 + SamplingState.SampleIndex * 401;
	FVector2 Sample = CMJ(Hash % CMJ_TOTAL_GRID_SIZE, CMJ_GRID_LINEAR_SIZE, CMJ_GRID_LINEAR_SIZE, Hash);
	float Theta = Sample.X * M_PI * 2;
	float R = sqrt(Sample.Y);
	Sample.X = R * cos(Theta);
	Sample.Y = R * sin(Theta);
	return Sample;
}

float RandomFloat(inout FSamplingState SamplingState)
{
	uint32_t Hash = 227 + SamplingState.Seed * 1489 + SamplingState.Bounce * 1399 + SamplingState.SampleIndex * 401;
	SamplingState.SampleIndex += 1;
	return RandomFloat(0, Hash);
}

#endif // CMJ_H