#pragma once
#include <string>

bool compressLZ(const std::string& inputFile, const std::string& outputFile);
bool decompressLZ(const std::string& inputFile, const std::string& outputFile);
