#include "ImageProcessor.hpp"
#include <Windows.h>
#include <onnxruntime_cxx_api.h>
#include <filesystem>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "Logger.hpp"
#ifdef _WIN32
#include <windows.h>  // For MultiByteToWideChar
namespace
{
std::wstring Utf8ToWString(const std::string& utf8String)
{
    if (utf8String.empty())
    {
        return std::wstring();
    }
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &utf8String[0], (int)utf8String.size(), NULL, 0);
    std::wstring wstrTo(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, &utf8String[0], (int)utf8String.size(), &wstrTo[0], sizeNeeded);
    return wstrTo;
}

std::vector<char> readFileToBuffer(const std::string& path)
{
    std::wstring wpath = Utf8ToWString(path);

    std::ifstream file(wpath, std::ios::binary);
    if (!file.is_open())
    {
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);

    return buffer;
}
}  // namespace
#endif

ImageProcessor::ImageProcessor(const std::string& path)
    : m_strImgPath(path)
{
}

ImageProcessor::~ImageProcessor()
{
}

bool ImageProcessor::processImageByAI(const STMaskRegion& maskRegion)
{
    const int MODEL_INPUT_H = 512;
    const int MODEL_INPUT_W = 512;
    const char* input_image_name = "image";
    const char* input_mask_name = "mask";
    const char* output_result_name = "output";

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
    cv::Mat src;
#ifdef _WIN32
    std::vector<char> buffer = readFileToBuffer(m_strImgPath);

    if (buffer.empty())
    {
        LOG_ERROR << "无法读取文件到内存: " << m_strImgPath << std::endl;
        return false;
    }

    src = cv::imdecode(buffer, cv::IMREAD_COLOR);

    if (src.empty())
    {
        LOG_ERROR << "Failed to read image from path: " << m_strImgPath;
        return false;
    }
#else
    src = cv::imread(m_strImgPath);
#endif

      // --- Smart Cropping Logic Start ---
    int padding = 64; 
    int x = std::max(0, maskRegion.x - padding);
    int y = std::max(0, maskRegion.y - padding);
    int w = maskRegion.w + 2 * padding;
    int h = maskRegion.h + 2 * padding;

    // Adjust cropping rect to not exceed image boundaries
    x = std::min(x, src.cols - 1);
    y = std::min(y, src.rows - 1);
    w = std::min(w, src.cols - x);
    h = std::min(h, src.rows - y);

    cv::Rect roi(x, y, w, h);
    cv::Mat cropped_src = src(roi).clone();

    // Create a mask for the cropped area
    cv::Mat cropped_mask = cv::Mat::zeros(cropped_src.size(), CV_8U);
    cv::Rect mask_roi_in_cropped_img(maskRegion.x - x, maskRegion.y - y, maskRegion.w, maskRegion.h);
    cv::rectangle(cropped_mask, mask_roi_in_cropped_img, cv::Scalar(255), -1);

    // Resize cropped image and mask to model input size
    cv::Mat resized_src, resized_mask;
    cv::resize(cropped_src, resized_src, cv::Size(MODEL_INPUT_W, MODEL_INPUT_H), 0, 0, cv::INTER_AREA);
    cv::resize(cropped_mask, resized_mask, cv::Size(MODEL_INPUT_W, MODEL_INPUT_H), 0, 0, cv::INTER_NEAREST);
    // --- Smart Cropping Logic End ---

    std::vector<float> input_image_data = preprocess_image(resized_src, MODEL_INPUT_H, MODEL_INPUT_W);
    std::vector<float> input_mask_data = preprocess_mask(resized_mask, MODEL_INPUT_H, MODEL_INPUT_W);

    std::vector<int64_t> image_input_shape = {1, 3, MODEL_INPUT_H, MODEL_INPUT_W};
    std::vector<int64_t> mask_input_shape = {1, 1, MODEL_INPUT_H, MODEL_INPUT_W};

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
    model_input_names_ordered.resize(2);
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
        for (int h_idx = 0; h_idx < out_h; ++h_idx)
        {
            for (int w_idx = 0; w_idx < out_w; ++w_idx)
            {
                output_image.at<cv::Vec3b>(h_idx, w_idx)[c] =
                    static_cast<unsigned char>(output_data[c * out_h * out_w + h_idx * out_w + w_idx]);
            }
        }
    }

    cv::cvtColor(output_image, output_image, cv::COLOR_RGB2BGR);

    // --- Post-processing and Splicing ---
    // Resize the output back to the cropped image's original size
    cv::resize(output_image, output_image, cropped_src.size(), 0, 0, cv::INTER_LANCZOS4);

    // Apply the restored region back to the original source image
    cv::Mat final_mask = cv::Mat::zeros(src.size(), CV_8U);
    cv::rectangle(final_mask, cv::Rect(maskRegion.x, maskRegion.y, maskRegion.w, maskRegion.h), cv::Scalar(255), -1);

    output_image.copyTo(src(roi), cropped_mask);

    cv::imwrite("output.png", src);

    return true;
}

std::vector<float> ImageProcessor::preprocess_image(const cv::Mat& img, int target_h, int target_w)
{
    cv::Mat resized_img;
    cv::resize(img, resized_img, cv::Size(target_w, target_h), 0, 0, cv::INTER_AREA);

    cv::cvtColor(resized_img, resized_img, cv::COLOR_BGR2RGB);

    resized_img.convertTo(resized_img, CV_32FC3, 1.0 / 255.0);

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
    cv::resize(mask, resized_mask, cv::Size(target_w, target_h), 0, 0, cv::INTER_NEAREST);

    if (resized_mask.channels() == 3)
    {
        cv::cvtColor(resized_mask, resized_mask, cv::COLOR_BGR2GRAY);
    }

    cv::threshold(resized_mask, resized_mask, 128, 255, cv::THRESH_BINARY);
    resized_mask.convertTo(resized_mask, CV_32FC1, 1.0 / 255.0);

    std::vector<float> data(1 * 1 * target_h * target_w);
    for (int h = 0; h < target_h; ++h)
    {
        for (int w = 0; w < target_w; ++w)
        {
            data[h * target_w + w] = resized_mask.at<float>(h, w);
        }
    }
    return data;
}