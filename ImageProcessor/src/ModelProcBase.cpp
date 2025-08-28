#include "ModelProcBase.hpp"
#include <codecvt>
#include <locale>

ModelProcBase::ModelProcBase(const std::string& name)
    : m_env(ORT_LOGGING_LEVEL_WARNING, name.c_str())
{
    m_sessionOptions.SetIntraOpNumThreads(1);
    m_sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_EXTENDED);
}

ModelProcBase::~ModelProcBase()
{
}

bool ModelProcBase::initModel(const std::string& path)
{
    m_model.clear();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wModelpath = converter.from_bytes(path);
    auto session = std::make_unique<Ort::Session>(m_env, wModelpath.c_str(), m_sessionOptions);
    Ort::AllocatorWithDefaultOptions allocator;
    std::vector<const char*> inputNames, outputNames;
    for (size_t i = 0; i < session->GetInputCount(); ++i)
        inputNames.push_back(session->GetInputNameAllocated(i, allocator).get());
    for (size_t i = 0; i < session->GetOutputCount(); ++i)
        outputNames.push_back(session->GetOutputNameAllocated(i, allocator).get());

    m_model = {std::move(session), inputNames, outputNames, path};

    return true;
}

std::vector<Ort::Value>&& ModelProcBase::getOutput()
{
    return std::move(m_output);
}