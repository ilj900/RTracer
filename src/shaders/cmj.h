#ifndef CMJ_H
#define CMJ_H

#define CMJ_GRID_LINEAR_SIZE 4
#define CMJ_TOTAL_GRID_SIZE (CMJ_GRID_LINEAR_SIZE * CMJ_GRID_LINEAR_SIZE)

struct FSamplingState
{
	uint Seed;
	uint Bounce;
	uint SampleIndex;
};

uint Permute(uint I, uint L, uint P)
{
	uint W = L - 1;
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

float RandomFloat(uint I, uint P)
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

vec2 CMJ(uint S, uint M, uint N, uint P)
{
	uint SX = Permute(S % M, M, P * 0xa511e9b3);
	uint SY = Permute(S / M, N, P * 0x63d83595);
	float JX = RandomFloat(S, P * 0xa399d265);
	float JY = RandomFloat(S, P * 0x711ad6a5);

	vec2 Result = {(S % M + (SY + JX) / N) / M, (S / M + (SX + JY) /M) / N};

	return Result;
}

vec2 Sample2D(FSamplingState SamplingState)
{
	uint Hash = 227 + SamplingState.Seed * 1489 + SamplingState.Bounce * 1399 + SamplingState.SampleIndex * 401;
	return CMJ(Hash % CMJ_TOTAL_GRID_SIZE, CMJ_GRID_LINEAR_SIZE, CMJ_GRID_LINEAR_SIZE, Hash);
}

float RandomFloat(inout FSamplingState SamplingState)
{
	uint Hash = 227 + SamplingState.Seed * 1489 + SamplingState.Bounce * 1399 + SamplingState.SampleIndex * 401;
	SamplingState.SampleIndex += 1;
	return RandomFloat(0, Hash);
}

#endif // CMJ_H