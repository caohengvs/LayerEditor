#pragma once
/**
 * @file: ImageProcessor.hpp
 * @brief: file brief description
 * @author: Curtis
 * @date: 2025-07-22 09:07:45
 * @version: 1.0
 * @email: caohengvs888@gmail.com
 */

#include <string>
#include "CommonDef/ExportDef.h"
namespace cv
{
class Mat;
};  // namespace cv

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
    signed char processImage();

private:
    void fftshift(const cv::Mat& inputImg, cv::Mat& outputImg);
   

private:
    std::string m_strImgPath;  // 默认路径
};