#pragma once

#include <string>

size_t FindString(const std::string& StringToCheck, const std::string& StringToFind, size_t StartingChar = 0);
std::string GetStringTillTheEOL(const std::string& StringToCheck, size_t StartingChar);
std::string GetIncludeFileName(const std::string& StringToCheck, size_t StartingChar);
size_t ReplaceString(std::string& StringToModify, const std::string& StringToBeReplaced, const std::string& StringToUseAsReplacements, size_t StartingChar = 0);
size_t RemoveString(std::string& StringToModify, const std::string& StringToRemove, size_t StartingChar = 0);
size_t InsertString(std::string& StringToModify, size_t Index, const std::string& StringToInsert);
std::string ExtractFileName(const std::string& Path);