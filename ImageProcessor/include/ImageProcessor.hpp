#pragma once
#include <string>
#include "CommonDef/ExportDef.h"
class LIB_API ImageProcessor
{
public:
    ImageProcessor() = default;
    ~ImageProcessor() = default;
    int processImage(const std::string& imagePath);
    void saveImage(const std::string& outputPath);
};