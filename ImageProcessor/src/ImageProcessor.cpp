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