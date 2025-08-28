/**
 * @file: ModelProcImage.hpp
 * @brief: 图像模型处理类
 * @author: Curtis Cao
 * @date: 2025-08-27 16:59:23
 * @version: 1.0
 * @email: caohengvs888@gmail.com
 */
#pragma once
#include <opencv2/opencv.hpp>
#include <optional>
#include "ModelProcBase.hpp"

class ModelProcImage final : public ModelProcBase
{
public:
    enum class EInputType
    {
        Image = 0,
        Mask
    };

public:
    explicit ModelProcImage(const std::string& name);
    ~ModelProcImage();

public:
    virtual bool run() override;

public:
    /**
     * @name: addSrc
     * @brief: 设置模型输入数据
     * @param: type:数据类型;input:原始数据
     * @return: 无
     */
    void addSrc(EInputType type, const std::vector<float>& input);

    void addShape(EInputType type, const std::vector<int64_t>& shape);

    template<typename T>
    std::optional<std::reference_wrapper<const T>> getVector(EInputType type,
                                                             const std::unordered_map<EInputType, T>& map) const
    {
        auto it = map.find(type);
        if (it != map.end())
        {
            return std::make_optional(std::cref(it->second));
        }
        return std::nullopt;
    }

private:
    bool isValid();

private:
    std::unordered_map<EInputType, std::vector<float>> m_srcData;
    std::unordered_map<EInputType, std::vector<int64_t>> m_srcShape;
};
