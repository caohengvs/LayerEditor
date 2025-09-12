#pragma once
#include "IHfFeCommonLayer.h"

class CHfFeCommonPcie final : public IHfFeCommonLayer
{
public:
    CHfFeCommonPcie();
    ~CHfFeCommonPcie() = default;

public:
    virtual bool Init(uint16_t nMaxCacheNodesNumber) override;
    virtual int Reset() override;
    virtual bool Open() override;
    virtual void Close() override;
    virtual bool ReadRegister(uint16_t nMemoryBarID, uint32_t nAddress, uint64_t& nValue) override;
    virtual bool WriteRegister(uint16_t nMemoryBarID, uint32_t nAddress, uint64_t nValue) override;
    // 获取DMA上传数据
    virtual uint32_t GetBuffer(void* pBuf, uint32_t nSize) override;

private:
    int enumDevice();
    bool openDevice();

private:
    std::vector<CString> m_arrDevicePath;
    HANDLE m_hDevice;
};
