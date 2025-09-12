#pragma once
#include <functional>
#include <future>
#include "HfFeCommonDefine.h"

class _HF_FECOMMONLAYER_DLL_HEADER_ IHfFeCommonLayer
{
    using GetBuffCallback = std::function<bool(void* pBuf, uint64_t nSize)>;

public:
    static IHfFeCommonLayer& s_GetInstance();

protected:
    IHfFeCommonLayer();
    virtual ~IHfFeCommonLayer();

public:
    virtual int Reset() = 0;
    virtual bool Open() = 0;
    virtual void Close() = 0;
    virtual bool ReadRegister(uint16_t, uint32_t, uint64_t&) = 0;
    virtual bool WriteRegister(uint16_t, uint32_t, uint64_t) = 0;
    virtual bool GetBuffer(void*, uint32_t) = 0;

public:
    virtual bool InitCollect(uint32_t);
    bool StartCollect(GetBuffCallback, uint16_t);
    bool StopCollect();
    bool isStop();

private:
    std::future<void> m_futCollect;
    std::atomic<bool> m_bRun;
    uint8_t* m_pCollectBuff;  // 收包缓存
    uint32_t m_nCollectSize;  // 收包大小
};
