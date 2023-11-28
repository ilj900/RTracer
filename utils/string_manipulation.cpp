#include "string_manipulation.h"

size_t FindString(const std::string& StringToCheck, const std::string& StringToFind, size_t StartingChar)
{
    return StringToCheck.find(StringToFind);
}

std::string GetStringTillTheEOL(const std::string& StringToCheck, size_t StartingChar)
{
    auto StartingIndex = StartingChar;
    size_t Length = 0;

    while (StringToCheck[StartingChar] != '\r')
    {
        StartingChar++;
        Length++;
    }

    std::string Result = StringToCheck.substr(StartingIndex, Length);
    return Result;
}

std::string GetIncludeFileName(const std::string& StringToCheck, size_t StartingChar)
{
    while (StringToCheck[StartingChar] != '"')
    {
        StartingChar++;
    }

    StartingChar++;
    size_t StartingIndex = StartingChar;
    size_t Length = 0;

    while (StringToCheck[StartingChar] != '"')
    {
        StartingChar++;
        Length++;
    }

    std::string Result = StringToCheck.substr(StartingIndex, Length);

    return Result;
}

size_t ReplaceString(std::string& StringToModify, const std::string& StringToBeReplaced, const std::string& StringToUseAsReplacements, size_t StartingChar)
{
    auto CharIndex = RemoveString(StringToModify, StringToBeReplaced);

    if (CharIndex != std::string::npos)
    {
        InsertString(StringToModify, CharIndex, StringToUseAsReplacements);
        return CharIndex;
    }

    return std::string::npos;
}

size_t RemoveString(std::string& StringToModify, const std::string& StringToRemove, size_t StartingChar)
{
    auto CharIndex = FindString(StringToModify, StringToRemove);

    if (CharIndex != std::string::npos)
    {
        StringToModify.erase(CharIndex, StringToRemove.size());
        return CharIndex;
    }

    return std::string::npos;
}

size_t InsertString(std::string& StringToModify, size_t Index, const std::string& StringToInsert)
{
    StringToModify.insert(Index, StringToInsert);

    return Index;
}

std::string ExtractFileName(const std::string& Path)
{
    std::string Result;
    bool bDotFound = false;

    for (int i = Path.size() - 1; i >=0; --i)
    {
        if (Path[i] == '.')
        {
            bDotFound = true;
        }

        if (Path[i] == '/' || Path[i] == '\\')
        {
            if (bDotFound)
            {
                std::reverse(Result.begin(), Result.end());
                return Result;
            }
        }

        Result.push_back(Path[i]);
    }

    return Result;
}
