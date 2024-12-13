#pragma once

#include <cstdint>
#include <string>

uint32_t CalculateGroupCount(uint32_t NumberOfElements, uint32_t ElementsInGroup);

uint32_t CalculateMaxGroupCount(uint32_t NumberOfElements, uint32_t ElementsInGroup);

void AddPrecedingZeroes(std::string& s, int ZeroesCount);
