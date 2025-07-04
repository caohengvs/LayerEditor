#pragma once
#include <string>
#include "CommonDef/ExportDef.h"
class LIB_API ImageProcessor
{
public:
    explicit ImageProcessor(const std::string& path);
    ~ImageProcessor();

public:
    int processImage(const std::string& path);
    bool processImage();

private:
    std::string m_strImgPath;
};