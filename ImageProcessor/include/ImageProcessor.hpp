#pragma once
/**
 * @file: ImageProcessor.hpp
 * @brief: 图像处理器类
 * @author: Curtis
 * @date: 2025-07-22 09:07:45
 * @version: 1.0
 * @email: caohengvs888@gmail.com
 */

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

    bool processImage(const STMaskRegion& maskRegion);
    bool processImage();
    bool processImageByAI(const STMaskRegion& maskRegion);

private:
    void fftshift(const cv::Mat& inputImg, cv::Mat& outputImg);

    // 图像预处理函数
    // 将 OpenCV Mat 转换为 ONNX 模型所需的 NCHW 浮点张量
    std::vector<float> preprocess_image(const cv::Mat& img, int target_h, int target_w);

    // 掩码预处理函数
    // 将 OpenCV Mat 转换为 ONNX 模型所需的 (1, 1, H, W) 浮点张量
    std::vector<float> preprocess_mask(const cv::Mat& mask, int target_h, int target_w);

private:
    std::string m_strImgPath;  // 默认路径
};