#include "pch.h"
#include "CHfFeDataParserBase.h"
#include "LocalDefine.h"

CHfFeDataParserBase::CHfFeDataParserBase()
    : m_raw(nullptr)
    , m_rawSize(0)
{
    m_raw = new uint8_t[PER_PACKAGE_MAXSIZE];
}

CHfFeDataParserBase::~CHfFeDataParserBase()
{
    if (m_raw)
    {
        delete[] m_raw;
        m_raw = nullptr;
    }
}

bool CHfFeDataParserBase::SetRawData(void* pRawData, uint32_t nRawSize)
{
    if (!pRawData || nRawSize > PER_PACKAGE_MAXSIZE || nRawSize <= 0)
    {
        return false;
    }
    std::fill_n(m_raw, PER_PACKAGE_MAXSIZE, uint8_t{0});

    memcpy_s(m_raw, nRawSize, pRawData, nRawSize);
    m_rawSize = nRawSize;

    return true;
}
