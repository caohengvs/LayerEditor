#include "ModelProcBase.hpp"
#include <boost/locale.hpp>
#include <filesystem>
#include "Logger.hpp"

ModelProcBase::ModelProcBase(const std::string& name)
    : m_env(ORT_LOGGING_LEVEL_WARNING, name.c_str())
{
    m_sessionOptions.SetIntraOpNumThreads(1);
    m_sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_EXTENDED);
}

ModelProcBase::~ModelProcBase()
{
    m_model.clear();
}

bool ModelProcBase::initModel(const std::string& path)
{
    if (!std::filesystem::exists(path))
    {
        LOG_ERROR << "model is not find:" << path;
        return false;
    }
    m_model.clear();
    std::wstring wModelpath = boost::locale::conv::to_utf<wchar_t>(path, "UTF-8");
    auto session = std::make_unique<Ort::Session>(m_env, wModelpath.c_str(), m_sessionOptions);
    Ort::AllocatorWithDefaultOptions allocator;
    std::vector<const char*> inputNames, outputNames;
    for (size_t i = 0; i < session->GetInputCount(); ++i)
    {
        auto ort_name_ptr = session->GetInputNameAllocated(i, allocator);
        const char* name = ort_name_ptr.get();

        char* new_name = new char[strlen(name) + 1];
        strcpy(new_name, name);
        inputNames.push_back(new_name);
    }
    for (size_t i = 0; i < session->GetOutputCount(); ++i)
    {
        auto ort_name_ptr = session->GetOutputNameAllocated(i, allocator);
        const char* name = ort_name_ptr.get();

        char* new_name = new char[strlen(name) + 1];
        strcpy(new_name, name);
        outputNames.push_back(new_name);
    }

    m_model = {std::move(session), inputNames, outputNames, path};

    return true;
}

std::vector<Ort::Value>&& ModelProcBase::getOutput()
{
    return std::move(m_output);
}