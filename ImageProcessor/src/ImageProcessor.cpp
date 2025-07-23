#include "ImageProcessor.hpp"
#include <onnxruntime_cxx_api.h>
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

bool ImageProcessor::processImage(const STMaskRegion& maskRegion)
{
    cv::Mat src = cv::imread(m_strImgPath);

    if (src.empty())
    {
        return false;
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

    return true;
}

bool ImageProcessor::processImage()
{
    cv::Mat src = cv::imread(m_strImgPath, cv::IMREAD_GRAYSCALE);
    if (src.empty())
    {
        return false;
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
    auto funcSync = [](cv::Mat& inputOutput_H, cv::Point center, int radius)
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
    return true;
}

bool ImageProcessor::processImageByAI(const STMaskRegion& maskRegion)
{
    const int MODEL_INPUT_H = 512;
    const int MODEL_INPUT_W = 512;
    const char* input_image_name = "image";     // 实际模型的图像输入名称
    const char* input_mask_name = "mask";       // 实际模型的掩码输入名称
    const char* output_result_name = "output";  // 实际模型的输出名称

    Ort::Env env(ORT_LOGGING_LEVEL_FATAL, "LamaCleanerInference");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(ORT_ENABLE_EXTENDED);
    std::wstring w_onnx_model_path(L"models/lama_fp32.onnx");
    Ort::Session session(env, w_onnx_model_path.c_str(), session_options);

    std::vector<Ort::AllocatedStringPtr> input_name_ptrs;
    std::vector<const char*> input_node_names;
    Ort::AllocatorWithDefaultOptions allocator;
    for (size_t i = 0; i < session.GetInputCount(); ++i)
    {
        input_name_ptrs.push_back(session.GetInputNameAllocated(i, allocator));
        input_node_names.push_back(input_name_ptrs.back().get());
    }

    std::vector<Ort::AllocatedStringPtr> output_name_ptrs;
    std::vector<const char*> output_node_names;
    for (size_t i = 0; i < session.GetOutputCount(); ++i)
    {
        output_name_ptrs.push_back(session.GetOutputNameAllocated(i, allocator));
        output_node_names.push_back(output_name_ptrs.back().get());
    }

    int image_input_idx = -1;
    int mask_input_idx = -1;
    for (size_t i = 0; i < input_node_names.size(); ++i)
    {
        if (std::string(input_node_names[i]) == input_image_name)
        {
            image_input_idx = i;
        }
        else if (std::string(input_node_names[i]) == input_mask_name)
        {
            mask_input_idx = i;
        }
    }

    if (image_input_idx == -1 || mask_input_idx == -1)
    {
        return false;
    }

    cv::Mat src = cv::imread(m_strImgPath);

    if (src.empty())
    {
        LOG_ERROR << "Failed to read image from path: " << m_strImgPath;
        return false;
    }

    cv::Mat mask = cv::Mat::zeros(src.size(), CV_8U);
    auto [x, y, w, h] = maskRegion;
    cv::Rect watermark_roi(x, y, w, h);
    cv::rectangle(mask, watermark_roi, cv::Scalar(255), -1);

    std::vector<float> input_image_data = preprocess_image(src, MODEL_INPUT_H, MODEL_INPUT_W);
    std::vector<float> input_mask_data = preprocess_mask(mask, MODEL_INPUT_H, MODEL_INPUT_W);

    std::vector<int64_t> image_input_shape = {1, 3, MODEL_INPUT_H, MODEL_INPUT_W};
    std::vector<int64_t> mask_input_shape = {1, 1, MODEL_INPUT_H, MODEL_INPUT_W};  // 掩码是单通道

    Ort::MemoryInfo memory_info =
        Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    std::vector<Ort::Value> input_tensors;
    input_tensors.reserve(2);
    input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, input_image_data.data(),
                                                            input_image_data.size(), image_input_shape.data(),
                                                            image_input_shape.size()));
    input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, input_mask_data.data(), input_mask_data.size(),
                                                            mask_input_shape.data(), mask_input_shape.size()));

    std::vector<const char*> model_input_names_ordered;
    model_input_names_ordered.reserve(2);
    model_input_names_ordered[image_input_idx] = input_image_name;
    model_input_names_ordered[mask_input_idx] = input_mask_name;

    LOG_DEBUG << "Running inference with ONNX Runtime...";
    auto output_tensors = session.Run(Ort::RunOptions{nullptr}, model_input_names_ordered.data(), input_tensors.data(),
                                      input_tensors.size(), output_node_names.data(), output_node_names.size());

    LOG_DEBUG << "Running inference with ONNX Runtime over...";

    float* output_data = output_tensors[0].GetTensorMutableData<float>();
    std::vector<int64_t> output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();

    int out_c = output_shape[1];
    int out_h = output_shape[2];
    int out_w = output_shape[3];

    cv::Mat output_image(out_h, out_w, CV_8UC3);
    for (int c = 0; c < out_c; ++c)
    {
        for (int h = 0; h < out_h; ++h)
        {
            for (int w = 0; w < out_w; ++w)
            {
                output_image.at<cv::Vec3b>(h, w)[c] =
                    static_cast<unsigned char>(output_data[c * out_h * out_w + h * out_w + w]);
            }
        }
    }

    cv::cvtColor(output_image, output_image, cv::COLOR_RGB2BGR);
    cv::resize(output_image, output_image, src.size(), 0, 0, cv::INTER_LINEAR);

    cv::imshow("Processed Image", output_image);
    cv::waitKey(0);
    cv::destroyAllWindows();
    // cv::imwrite("output_image.png", output_image);

    return true;
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

std::vector<float> ImageProcessor::preprocess_image(const cv::Mat& img, int target_h, int target_w)
{
    cv::Mat resized_img;
    cv::resize(img, resized_img, cv::Size(target_w, target_h), 0, 0, cv::INTER_AREA);

    // OpenCV 默认是 BGR，ONNX 模型通常期望 RGB
    cv::cvtColor(resized_img, resized_img, cv::COLOR_BGR2RGB);

    // 转换为 float 类型并归一化到 [-1, 1]
    resized_img.convertTo(resized_img, CV_32FC3, 1.0 / 255.0);
    // resized_img.convertTo(resized_img, CV_32FC3, 1.0 / 127.5, -1.0);  // 归一化到 [-1, 1]

    // HWC to CHW
    std::vector<float> data(1 * 3 * target_h * target_w);
    for (int c = 0; c < 3; ++c)
        for (int h = 0; h < target_h; ++h)
            for (int w = 0; w < target_w; ++w)
                data[c * target_h * target_w + h * target_w + w] = resized_img.at<cv::Vec3f>(h, w)[c];
    return data;
}

std::vector<float> ImageProcessor::preprocess_mask(const cv::Mat& mask, int target_h, int target_w)
{
    cv::Mat resized_mask;
    cv::resize(mask, resized_mask, cv::Size(target_w, target_h), 0, 0, cv::INTER_NEAREST);  // 最近邻插值保持二值性

    // 确保掩码是单通道灰度图
    if (resized_mask.channels() == 3)
    {
        cv::cvtColor(resized_mask, resized_mask, cv::COLOR_BGR2GRAY);
    }

    // 二值化（如果不是纯黑白）并转换为 float 类型归一化到 [0, 1]
    cv::threshold(resized_mask, resized_mask, 128, 255, cv::THRESH_BINARY);
    resized_mask.convertTo(resized_mask, CV_32FC1, 1.0 / 255.0);  // 正确归一化到[0,1]

    // HWC (Height, Width, Channel) to CHW (Channel, Height, Width) - 掩码只有一个通道
    std::vector<float> data(1 * 1 * target_h * target_w);  // Batch size 1, 1 channel
    for (int h = 0; h < target_h; ++h)
    {
        for (int w = 0; w < target_w; ++w)
        {
            // data[0 * H * W + h * W + w]
            data[h * target_w + w] = resized_mask.at<float>(h, w);
        }
    }
    return data;
}
