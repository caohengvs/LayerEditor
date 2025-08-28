/**
 * @file: ModelProcBase.hpp
 * @brief: 模型处理基类
 * @author: Curtis Cao
 * @date: 2025-08-27 15:46:52
 * @version: 1.0
 * @email: caohengvs888@gmail.com
 */
#pragma once
// #include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <string>
// #include <vector>
// #include <map>
// #include <functional>
// #include <future>
// #include <memory>
// #include <variant>
class ModelProcBase
{
    struct ModelData
    {
        std::unique_ptr<Ort::Session> session;
        std::vector<const char*> inputNames;
        std::vector<const char*> outputNames;
        std::string modelPath;

        void clear()
        {
            session.reset();
            for (auto name : inputNames)
            {
                delete[] name;
            }
            for (auto name : outputNames)
            {
                delete[] name;
            }
            inputNames.clear();
            outputNames.clear();
            modelPath.clear();
        }

        bool isValid()
        {
            return session.get() && !inputNames.empty() && !outputNames.empty();
        }
    };

public:
    /**
     * @name: ModelProcBase
     * @brief: 构造函数
     * @param: name：模型名称
     * @return: 无
     */
    explicit ModelProcBase(const std::string& name);
    ~ModelProcBase();

    ModelProcBase(const ModelProcBase&) = delete;
    ModelProcBase& operator=(const ModelProcBase&) = delete;
    ModelProcBase(ModelProcBase&&) = delete;
    ModelProcBase& operator=(ModelProcBase&&) = delete;

public:
    /**
    * @name: initModel
    * @brief: 初始化模型参数
    * @param: path:模型路径
    * @return: true:初始化成功;false:初始化失败
    */
    bool initModel(const std::string& path);

    virtual bool run() = 0;

    /**
     * @name: getOutput
     * @brief: 获取执行结果,只允许获取一次
     * @param: 无
     * @return: 张量数据
     */
    std::vector<Ort::Value>&& getOutput();

protected:
    Ort::Env m_env;                        // onnx当前环境
    Ort::SessionOptions m_sessionOptions;  // onnx当前会话选项
    ModelData m_model;                     // 当前model
    std::vector<Ort::Value> m_output;      // 运行结果
};