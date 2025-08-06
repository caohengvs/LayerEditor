#pragma once
/**
 * @file: ImageProcessor.hpp
 * @brief: 图像处理器类
 * @author: Curtis
 * @date: 2025-07-22 09:07:45
 * @version: 1.0
 * @email: caohengvs888@gmail.com
 */
#include <optional>
#include <string>
#include <vector>
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

    bool processImageByAI(const STMaskRegion& maskRegion);

private:
    void fftshift(const cv::Mat& inputImg, cv::Mat& outputImg);

    std::vector<float> preprocess_image(const cv::Mat& img, int target_h, int target_w);

    std::vector<float> preprocess_mask(const cv::Mat& mask, int target_h, int target_w);

private:
    std::string m_strImgPath;  // 默认路径
};