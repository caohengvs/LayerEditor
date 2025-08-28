#include "ModelProcImage.hpp"
#include "Logger.hpp"
ModelProcImage::ModelProcImage(const std::string& name)
    : ModelProcBase(name)
{
}

ModelProcImage::~ModelProcImage()
{
}

bool ModelProcImage::run()
{
    if (!m_model.isValid() || !isValid())
        return false;

    std::vector<float> input_image_data = getVector(EInputType::Image, m_srcData).value();
    std::vector<float> input_mask_data = getVector(EInputType::Mask, m_srcData).value();
    std::vector<int64_t> image_input_shape = getVector(EInputType::Image, m_srcShape).value();
    std::vector<int64_t> mask_input_shape = getVector(EInputType::Mask, m_srcShape).value();
    Ort::MemoryInfo memory_info =
        Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    std::vector<Ort::Value> input_tensors;
    input_tensors.reserve(2);
    input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, input_image_data.data(),
                                                            input_image_data.size(), image_input_shape.data(),
                                                            image_input_shape.size()));
    input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, input_mask_data.data(), input_mask_data.size(),
                                                            mask_input_shape.data(), mask_input_shape.size()));

    try
    {
        m_output =
            m_model.session.get()->Run(Ort::RunOptions{nullptr}, m_model.inputNames.data(), input_tensors.data(),
                                       input_tensors.size(), m_model.outputNames.data(), m_model.outputNames.size());
    }
    catch (const Ort::Exception& e)
    {
        LOG_ERROR << "code" << static_cast<int>(e.GetOrtErrorCode()) << e.what();
        return false;
    }

    return true;
}

void ModelProcImage::addSrc(EInputType type, const std::vector<float>& input)
{
    m_srcData.insert_or_assign(type, std::move(input));
}

void ModelProcImage::addShape(EInputType type, const std::vector<int64_t>& shape)
{
    m_srcShape.insert_or_assign(type, std::move(shape));
}

bool ModelProcImage::isValid()
{
    if (m_srcData.size() != m_srcShape.size() || m_srcData.empty())
        return false;

    const auto& optional = getVector(EInputType::Image, m_srcData);
    if (optional == std::nullopt)
    {
        return false;
    }

    const auto& optionalMask = getVector(EInputType::Mask, m_srcData);
    if (optional == std::nullopt)
    {
        return false;
    }

    const auto& optionalShape = getVector(EInputType::Image, m_srcShape);
    if (optional == std::nullopt)
    {
        return false;
    }

    const auto& optionalShapeMask = getVector(EInputType::Mask, m_srcShape);
    if (optional == std::nullopt)
    {
        return false;
    }

    return true;
}
