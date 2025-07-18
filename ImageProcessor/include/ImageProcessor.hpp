#pragma once
#include <string>
#include "CommonDef/ExportDef.h"
class LIB_API ImageProcessor
{
    struct STMaskRegion
    {
        int x; 
        int y; 
        int w; 
        int h; 
    };
    using PSTMaskRegion = STMaskRegion*;
    
public:
    explicit ImageProcessor(const std::string& path);
    ~ImageProcessor();

    signed char processImage(const STMaskRegion& maskRegion);

private:
    std::string m_strImgPath; // 默认路径
};