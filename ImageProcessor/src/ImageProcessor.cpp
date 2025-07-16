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

int ImageProcessor::processImage(const std::string& imagePath)
{

    cv::Scalar lower_watermark_color = cv::Scalar(0, 0, 200);
    cv::Scalar upper_watermark_color = cv::Scalar(179, 50, 255);
    

    int kernel_size = 5;        
    int dilate_iterations = 1;  
    int erode_iterations = 1;   

    int inpaintRadius = 3;  
    int inpaintAlgorithm = cv::INPAINT_TELEA;


    cv::Mat src = cv::imread(imagePath, cv::IMREAD_UNCHANGED);
    if (src.empty())
    {
        std::cerr << "Error: Could not open or find the image at " << imagePath << std::endl;
        return -1;
    }

    cv::Mat bgr_src;
    if (src.channels() == 4)
    {  
        cv::cvtColor(src, bgr_src, cv::COLOR_BGRA2BGR);
    }
    else
    {  
        bgr_src = src.clone();
    }

    cv::Mat hsv_src;
    cv::cvtColor(bgr_src, hsv_src, cv::COLOR_BGR2HSV);

    cv::Mat watermark_mask;
    cv::inRange(hsv_src, lower_watermark_color, upper_watermark_color, watermark_mask);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel_size, kernel_size));

    cv::morphologyEx(watermark_mask, watermark_mask, cv::MORPH_CLOSE, kernel);

    cv::dilate(watermark_mask, watermark_mask, kernel, cv::Point(-1, -1), dilate_iterations);

    // cv::erode(watermark_mask, watermark_mask, kernel, cv::Point(-1,-1), erode_iterations);

    cv::Mat dst_inpainted;
    cv::inpaint(src, watermark_mask, dst_inpainted, inpaintRadius, inpaintAlgorithm);

    cv::imshow("Original Image", src);
    cv::imshow("Generated Watermark Mask", watermark_mask);
    cv::imshow("Inpainted Result", dst_inpainted);

    cv::waitKey(0);
    cv::destroyAllWindows();

    // cv::imwrite("inpainted_image.png", dst_inpainted);

    return 0;
}

bool ImageProcessor::processImage()
{
    cv::Mat src = cv::imread(m_strImgPath, cv::IMREAD_UNCHANGED);
    if (src.empty())
    {
        LOG_DEBUG << "Error: Could not open or find the image at " << m_strImgPath;
        return false;
    }

    cv::Mat dst_exact;
    cv::Size newSize(800, 800);
    cv::resize(src, dst_exact, newSize, 0, 0, cv::INTER_LINEAR);
    cv::imshow("original imag", dst_exact);

    cv::waitKey(0);
    cv::destroyAllWindows();

    return true;
}
