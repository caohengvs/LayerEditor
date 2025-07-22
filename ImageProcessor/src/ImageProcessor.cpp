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

signed char ImageProcessor::processImage()
{
    cv::Mat src = cv::imread(m_strImgPath, cv::IMREAD_GRAYSCALE);
    if (src.empty())
    {
        return -1;
    }

    cv::Mat padded;
    int m = cv::getOptimalDFTSize(src.rows);  
    int n = cv::getOptimalDFTSize(src.cols);  
    
    cv::copyMakeBorder(src, padded, 0, m - src.rows, 0, n - src.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

   
    cv::Mat float_padded;
    padded.convertTo(float_padded, CV_32F);

   
    std::vector<cv::Mat> planes;
    planes.push_back(float_padded);                          
    planes.push_back(cv::Mat::zeros(padded.size(), CV_32F));  

    cv::Mat complexI;
    cv::merge(planes, complexI);  

    
    cv::dft(complexI, complexI);

    
    fftshift(complexI, complexI);

  
    cv::split(complexI, planes);  


    cv::Mat real_part = planes[0].clone();
    cv::Mat imag_part = planes[1].clone();

    cv::Mat magnitudeImage;
    cv::magnitude(real_part, imag_part, magnitudeImage);  

    
    magnitudeImage += cv::Scalar::all(1);  
    cv::log(magnitudeImage, magnitudeImage);

    
    cv::normalize(magnitudeImage, magnitudeImage, 0, 255, cv::NORM_MINMAX);
    magnitudeImage.convertTo(magnitudeImage, CV_8U);

    cv::imshow("Magnitude Spectrum (Identify Noise Spikes)", magnitudeImage);
    std::cout << "Please identify the coordinates of noise spikes in the 'Magnitude Spectrum' window." << std::endl;
    std::cout << "You will need to adjust the filter parameters in the code based on these locations." << std::endl;
    cv::waitKey(0);  

    
    cv::Mat H = cv::Mat::ones(complexI.size(), CV_32F);

    
    int notch_radius = 5;  
    auto funcSync =
        [](cv::Mat& inputOutput_H, cv::Point center, int radius)
    {
      
        cv::circle(inputOutput_H, center, radius, cv::Scalar(0), -1, 8);  

        cv::Point c2(center.x, inputOutput_H.rows - center.y);
        cv::Point c3(inputOutput_H.cols - center.x, center.y);
        cv::Point c4(inputOutput_H.cols - center.x, inputOutput_H.rows - center.y);

        cv::circle(inputOutput_H, c2, radius, cv::Scalar(0), -1, 8);
        cv::circle(inputOutput_H, c3, radius, cv::Scalar(0), -1, 8);
        cv::circle(inputOutput_H, c4, radius, cv::Scalar(0), -1, 8);
    };

   
    funcSync(H, cv::Point(complexI.cols / 2 + 50, complexI.rows / 2 + 50), notch_radius);
    funcSync(H, cv::Point(complexI.cols / 2 - 50, complexI.rows / 2 + 50), notch_radius);
    funcSync(H, cv::Point(complexI.cols / 2 + 50, complexI.rows / 2 - 50), notch_radius);
    funcSync(H, cv::Point(complexI.cols / 2 - 50, complexI.rows / 2 - 50), notch_radius);
 
    cv::Mat H_shifted;
    fftshift(H, H_shifted);

    std::vector<cv::Mat> planesH;
    planesH.push_back(H_shifted);
    planesH.push_back(cv::Mat::zeros(H_shifted.size(), CV_32F));
    cv::Mat complexH;
    cv::merge(planesH, complexH);

  
    cv::Mat complexIH;
    cv::mulSpectrums(complexI, complexH, complexIH, 0); 

    cv::Mat restoredComplexI;
   
    cv::idft(complexIH, restoredComplexI, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);

    
    cv::Mat restoredImage;
    restoredComplexI.convertTo(restoredImage, CV_8U);
    cv::normalize(restoredImage, restoredImage, 0, 255, cv::NORM_MINMAX);

    cv::imshow("Original Grayscale Image", src);
    cv::imshow("Restored Image (Frequency Filtered)", restoredImage);

    cv::waitKey(0);
    return 0;
}

void ImageProcessor::fftshift(const cv::Mat& inputImg, cv::Mat& outputImg)
{
    outputImg = inputImg.clone();
    int cx = outputImg.cols / 2;
    int cy = outputImg.rows / 2;

    cv::Mat q0(outputImg, cv::Rect(0, 0, cx, cy));    // Top-Left
    cv::Mat q1(outputImg, cv::Rect(cx, 0, cx, cy));   // Top-Right
    cv::Mat q2(outputImg, cv::Rect(0, cy, cx, cy));   // Bottom-Left
    cv::Mat q3(outputImg, cv::Rect(cx, cy, cx, cy));  // Bottom-Right

    cv::Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
}
