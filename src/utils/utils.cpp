#include "utils.h"

int CalculateGroupCount(int NumberOfElements, int ElementsInGroup)
{
    return (NumberOfElements + ElementsInGroup - 1) / ElementsInGroup;
}