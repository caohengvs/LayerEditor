#pragma once
#include <string>
#include "CommonDef/ExportDef.h"
class LIB_API ImageProcessor
{
public:
    explicit ImageProcessor(const std::string& path);
    ~ImageProcessor();

private:
    std::string m_strImgPath; // 默认路径
};