/**
 * @file: ImageProcessor.hpp
 * @brief: 图像处理器类
 * @author: Curtis Cao
 * @date: 2025-07-22 09:07:45
 * @version: 1.0
 * @email: caohengvs888@gmail.com
 */
#pragma once
#include <memory>
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
        STMaskRegion()
            : x(0)
            , y(0)
            , w(0)
            , h(0)
        {
        }
        STMaskRegion(int _x, int _y, int _w, int _h)
            : x(_x)
            , y(_y)
            , w(_w)
            , h(_h)
        {
        }
    };
    using PSTMaskRegion = STMaskRegion*;

public:
    struct STImageInfo
    {
        std::unique_ptr<unsigned char[]> buffer;
        int cols;
        int rows;
        int step;

        STImageInfo()
            : buffer(nullptr)
            , cols(0)
            , rows(0)
            , step(0)
        {
        }

        STImageInfo(unsigned char* _buffer, int _cols, int _rows, int _step)
            : buffer(std::move(_buffer))
            , cols(_cols)
            , rows(_rows)
            , step(_step)
        {
        }
    };
    using PSTImageInfo = STImageInfo*;

public:
    explicit ImageProcessor(const std::string& path);
    ~ImageProcessor();

public:
    bool processImageByAI(const STMaskRegion& maskRegion);
    bool processImageTest(const STMaskRegion& maskRegion);

    bool save(const std::string& path);

    bool showTest();

    std::optional<STImageInfo> getImageInfo();

private:
    std::vector<float> preprocess_image(const cv::Mat& img, int target_h, int target_w);

    std::vector<float> preprocess_mask(const cv::Mat& mask, int target_h, int target_w);

private:
    std::string m_strImgPath;           // 默认路径
    std::unique_ptr<cv::Mat> m_srcMat;  // 源图像
    std::unique_ptr<cv::Mat> m_outMat;  // 输出图像

    const int MODEL_INPUT_H = 512;
    const int MODEL_INPUT_W = 512;
};