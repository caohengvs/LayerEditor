#pragma once
#include <functional>
#include <future>
#include "HfFeCommonDefine.h"

class _HF_FECOMMONLAYER_DLL_HEADER_ IHfFeCommonLayer
{
    using GetBuffCallback = std::function<void(void* pBuf, uint32_t nSize)>;

public:
    static IHfFeCommonLayer& s_GetInstance();

protected:
    IHfFeCommonLayer();
    virtual ~IHfFeCommonLayer();

public:
    // 重置设备,软重置
    virtual bool Init(uint16_t) = 0;
    virtual int Reset() = 0;
    virtual bool Open() = 0;
    virtual void Close() = 0;
    virtual bool ReadRegister(uint16_t, uint32_t, uint64_t&) = 0;
    virtual bool WriteRegister(uint16_t, uint32_t, uint64_t) = 0;
    // 获取fpga上传数据
    virtual uint32_t GetBuffer(void*, uint32_t) = 0;

public:
    virtual bool InitCollect(uint32_t nBufferSize);
    bool StartCollect(GetBuffCallback callback, uint16_t nDurationMs);
    bool StopCollect();
    bool IsRun();

private:
    std::future<void> m_futCollect;
    std::atomic<bool> m_bRun;
    uint8_t* m_pCollectBuff;  // 收包缓存
    uint32_t m_nCollectSize;  // 收包大小
};
