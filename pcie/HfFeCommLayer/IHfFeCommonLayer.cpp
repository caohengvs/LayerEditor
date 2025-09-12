#include "pch.h"
#include <chrono>
#include "IHfFeCommonLayer.h"
#include "CHfFeCommonPcie.h"

IHfFeCommonLayer::IHfFeCommonLayer()
    : m_pCollectBuff(nullptr)
    , m_bRun(false)
    , m_nCollectSize(0)
{
}

IHfFeCommonLayer::~IHfFeCommonLayer()
{
    StopCollect();

    if (m_pCollectBuff)
    {
        free(m_pCollectBuff);
        m_pCollectBuff = nullptr;
    }
}

IHfFeCommonLayer& IHfFeCommonLayer::s_GetInstance()
{
    static CHfFeCommonPcie instance;
    return instance;
}

bool IHfFeCommonLayer::InitCollect(uint32_t nBufferSize)
{
    if (m_pCollectBuff)
    {
        free(m_pCollectBuff);
        m_pCollectBuff = nullptr;
    }

    m_pCollectBuff = reinterpret_cast<uint8_t*>(malloc(nBufferSize));
    return true;
}

bool IHfFeCommonLayer::StartCollect(GetBuffCallback callback, uint16_t nDurationMs)
{
    if (m_bRun.load() || !m_pCollectBuff)
    {
        return false;
    }

    m_bRun.store(true);
    m_futCollect = std::async(std::launch::async,
                              [this, callback, nDurationMs]()
                              {
                                  while (m_bRun.load())
                                  {
                                      memset(m_pCollectBuff, 0, m_nCollectSize);
                                      if (callback)
                                      {
                                          uint32_t rst = GetBuffer(m_pCollectBuff, m_nCollectSize);
                                          callback(m_pCollectBuff, rst);
                                      }

                                      std::this_thread::sleep_for(std::chrono::milliseconds(nDurationMs));
                                  }
                              });

    return true;
}

bool IHfFeCommonLayer::StopCollect()
{
    if (!m_bRun.load())
    {
        return false;
    }

    m_bRun.store(false);
    if (m_futCollect.valid())
    {
        m_futCollect.wait();
    }
    return true;
}

bool IHfFeCommonLayer::IsRun()
{
    return m_bRun.load();
}
