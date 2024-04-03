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