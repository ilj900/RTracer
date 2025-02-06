#include "utils.h"

#include "maths.h"

uint32_t CalculateGroupCount(uint32_t NumberOfElements, uint32_t ElementsInGroup)
{
    return (NumberOfElements + ElementsInGroup - 1) / ElementsInGroup;
}

uint32_t CalculateMaxGroupCount(uint32_t NumberOfElements, uint32_t ElementsInGroup)
{
    uint32_t GroupCount = (NumberOfElements + ElementsInGroup - 1) / ElementsInGroup;
    return 2 << Log2(GroupCount);
}

void AddPrecedingZeroes(std::string& String, int ZeroesCount)
{
	if (String.size() >= ZeroesCount)
	{
		return;
	}

	auto ZeroesToAdd = ZeroesCount - String.size();
	std::string Zeroes(ZeroesToAdd, '0');
	String = Zeroes + String;
}

float LinearToSRGB(float X)
{
	return (X <= 0.00311308f) ? (12.92f * X) : (1.055f * powf(X, 1.f / 2.4f) - 0.055f);
}
