#include "ImageProcessor.hpp"
#include <onnxruntime_cxx_api.h>
#include <boost/locale.hpp>
#include <filesystem>
#include <fstream>
#include <magic_enum/magic_enum.hpp>
#include <opencv2/opencv.hpp>
#include "Logger.hpp"
#include "ModelProcImage.hpp"

// #ifdef _WIN32
// #include <windows.h>  // For MultiByteToWideChar
// /**
//  * @name: Utf8ToWString
//  * @brief: win32API,string2wstring
//  * @param: 参数名 参数描述
//  * @return: 返回值描述
//  */
// std::wstring Utf8ToWString(const std::string& utf8String)
// {
//     if (utf8String.empty())
//     {
//         return std::wstring();
//     }
//     int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &utf8String[0], (int)utf8String.size(), NULL, 0);
//     std::wstring wstrTo(sizeNeeded, 0);
//     MultiByteToWideChar(CP_UTF8, 0, &utf8String[0], (int)utf8String.size(), &wstrTo[0], sizeNeeded);
//     return wstrTo;
// }
// #endif

namespace
{
std::vector<char> readFileToBuffer(const std::string& path)
{
    std::wstring wpath = boost::locale::conv::to_utf<wchar_t>(path, "UTF-8");
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

ImageProcessor::ImageProcessor()
{
}

ImageProcessor::~ImageProcessor()
{
}

bool ImageProcessor::processImageByAI(const std::string& path,const STMaskRegion& maskRegion)
{
    ModelProcImage modelImg("LamaCleanerInference");

    LOG_INFO << "Init model file:models/lama_fp32.onnx,begin.";
    if (!modelImg.initModel("models/lama_fp32.onnx"))
    {
        LOG_ERROR << "Init model file,failed.";
        return false;
    }

    LOG_INFO << "Init model file:models/lama_fp32.onnx,end.";
    cv::Mat src;
    std::vector<char> buffer = readFileToBuffer(path);

    if (buffer.empty())
    {
        LOG_ERROR << "无法读取文件到内存: " << path << std::endl;
        return false;
    }

    src = cv::imdecode(buffer, cv::IMREAD_COLOR);

    if (src.empty())
    {
        LOG_ERROR << "Failed to read image from path: " << path;
        return false;
    }

    int padding = 64;
    int x = std::max(0, maskRegion.x - padding);
    int y = std::max(0, maskRegion.y - padding);
    int w = maskRegion.w + 2 * padding;
    int h = maskRegion.h + 2 * padding;

    x = std::min(x, src.cols - 1);
    y = std::min(y, src.rows - 1);
    w = std::min(w, src.cols - x);
    h = std::min(h, src.rows - y);

    cv::Rect roi(x, y, w, h);
    cv::Mat cropped_src = src(roi).clone();

    cv::Mat cropped_mask = cv::Mat::zeros(cropped_src.size(), CV_8U);
    cv::Rect mask_roi_in_cropped_img(maskRegion.x - x, maskRegion.y - y, maskRegion.w, maskRegion.h);
    cv::rectangle(cropped_mask, mask_roi_in_cropped_img, cv::Scalar(255), -1);

    std::vector<float> input_image_data = preprocess_image(cropped_src, MODEL_INPUT_H, MODEL_INPUT_W);
    std::vector<float> input_mask_data = preprocess_mask(cropped_mask, MODEL_INPUT_H, MODEL_INPUT_W);

    modelImg.addSrc(ModelProcImage::EInputType::Image, input_image_data);
    modelImg.addSrc(ModelProcImage::EInputType::Mask, input_mask_data);

    modelImg.addShape(ModelProcImage::EInputType::Image, {1, 3, MODEL_INPUT_H, MODEL_INPUT_W});
    modelImg.addShape(ModelProcImage::EInputType::Mask, {1, 1, MODEL_INPUT_H, MODEL_INPUT_W});
    LOG_INFO << "Running inference with ONNX Runtime,beign.";

    if (!modelImg.run())
    {
        LOG_ERROR << "Running inference with ONNX Runtime,failed.";

        return false;
    }

    auto output_tensors = modelImg.getOutput();

    LOG_INFO << "Running inference with ONNX Runtime,end.";

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

    cv::resize(output_image, output_image, cropped_src.size(), 0, 0, cv::INTER_LANCZOS4);

    cv::Mat final_mask = cv::Mat::zeros(src.size(), CV_8U);
    cv::rectangle(final_mask, cv::Rect(maskRegion.x, maskRegion.y, maskRegion.w, maskRegion.h), cv::Scalar(255), -1);

    output_image.copyTo(src(roi), cropped_mask);

    m_outMat = std::make_unique<cv::Mat>(std::move(src));
    return true;
}

bool ImageProcessor::save(const std::string& path)
{
    if (!m_outMat)
    {
        return false;
    }

    cv::imwrite(path, *m_outMat.get());

    return true;
}

bool ImageProcessor::showTest()
{
    if (!m_outMat)
    {
        return false;
    }

    cv::imshow("outTest", *m_outMat.get());
    cv::waitKey(-1);
    cv::destroyAllWindows();

    return true;
}

std::optional<ImageProcessor::STImageInfo> ImageProcessor::getImageInfo()
{
    if (!m_outMat)
    {
        return std::nullopt;
    }
    cv::Mat rgb;
    cv::cvtColor(*m_outMat, rgb, cv::COLOR_BGR2RGB);
    STImageInfo stImg;

    size_t bufferSize = m_outMat->total() * m_outMat->elemSize();

    stImg.buffer.reset(new (std::nothrow) unsigned char[bufferSize]);
    if (!stImg.buffer)
    {
        return stImg;
    }

    memcpy_s(stImg.buffer.get(), bufferSize, rgb.data, bufferSize);

    stImg.cols = m_outMat->cols;
    stImg.rows = m_outMat->rows;
    stImg.step = m_outMat->step;

    return std::move(stImg);
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