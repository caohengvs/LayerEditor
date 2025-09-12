#pragma once
#include "HfFeDataParserDefine.h"

constexpr size_t PER_PACKAGE_MAXSIZE = 1024 * 1024 * 8;  // 8 MB
constexpr size_t PACKAGE_MAXNUM = 32;                    // 32 ²Û
constexpr size_t TOTAL_BYTES = PER_PACKAGE_MAXSIZE * PACKAGE_MAXNUM;

class _HF_FEDATAPARSER_DLL_HEADER_ CHfFeDataParserBase
{
public:
    CHfFeDataParserBase();
    virtual ~CHfFeDataParserBase();

public:
    bool SetRawData(void* pRawData, uint32_t nRawSize);

protected:
    uint8_t* m_raw;
    size_t m_rawSize;
};
