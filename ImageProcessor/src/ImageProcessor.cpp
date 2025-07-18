#include "ImageProcessor.hpp"
#include <filesystem>
#include <opencv2/opencv.hpp>
#include "Logger.hpp"

ImageProcessor::ImageProcessor(const std::string& path)
    : m_strImgPath(path)
{
}

ImageProcessor::~ImageProcessor()
{
}

signed char ImageProcessor::processImage(const STMaskRegion& maskRegion)
{
    cv::Mat src = cv::imread(m_strImgPath);

    if (src.empty())
    {
        return -1;
    }

    cv::Mat mask = cv::Mat::zeros(src.size(), CV_8U);  

    auto [x, y, w, h] = maskRegion;
    cv::Rect watermark_roi(x, y, w, h);  

    cv::rectangle(mask, watermark_roi, cv::Scalar(255), -1); 

    cv::Mat repaired_img;
    double inpaintRadius = 40;            
    int inpaintFlags = cv::INPAINT_NS;  
    cv::inpaint(src, mask, repaired_img, inpaintRadius, inpaintFlags);

    cv::imshow("repaired", repaired_img);

    cv::waitKey(0);

    cv::destroyAllWindows();

    return 0;
}
