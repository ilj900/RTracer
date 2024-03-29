#include "utils.h"

uint32_t CalculateGroupCount(uint32_t NumberOfElements, uint32_t ElementsInGroup)
{
    return (NumberOfElements + ElementsInGroup - 1) / ElementsInGroup;
}