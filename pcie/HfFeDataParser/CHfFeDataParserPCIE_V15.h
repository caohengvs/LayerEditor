#pragma once
#include <unordered_map>
#include "CHfFeDataParserBase.h"
#include "LocalDefine.h"
#pragma warning(push)
#pragma warning(disable : 4251)

class _HF_FEDATAPARSER_DLL_HEADER_ CHfFeDataParserPCIE_V15 : public CHfFeDataParserBase
{
    struct _ST_LINEINFO
    {
        size_t hdrOffset;  // 线头偏移
        size_t rfOffset;   // RF 数据偏移
        size_t rfSize;     // RF 数据长度（= linlen*16）
    };
    typedef _ST_LINEINFO StLineInfo;

public:
    CHfFeDataParserPCIE_V15();
    ~CHfFeDataParserPCIE_V15() = default;
    std::vector<StLineInfo> GetLineData();

private:
    std::vector<StLineInfo> m_lines;  // 一包里可能有多条线
};
#pragma warning(disable : 4251)
namespace
{

uint32_t swapEndian32(uint32_t value)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap32(value);
#elif defined(_MSC_VER)
    return _byteswap_ulong(value);
#else
    // 手动位操作作为回退选项
    return ((value >> 24) & 0xFF) | ((value << 8) & 0xFF0000) | ((value >> 8) & 0xFF00) | ((value << 24) & 0xFF000000);
#endif
}

inline uint16_t swapEndian16(uint16_t value)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap16(value);
#elif defined(_MSC_VER)
    return _byteswap_ushort(value);
#else
    // 手动位操作作为回退选项
    return ((value >> 8) & 0xFF) | ((value << 8) & 0xFF00);
#endif
}

inline uint64_t swapEndian64(uint64_t value)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap64(value);
#elif defined(_MSC_VER)
    return _byteswap_uint64(value);
#else
    /* 纯 C 回退方案 */
    return ((value >> 56) & 0x00000000000000FFULL) | ((value >> 40) & 0x000000000000FF00ULL) |
           ((value >> 24) & 0x0000000000FF0000ULL) | ((value >> 8) & 0x00000000FF000000ULL) |
           ((value << 8) & 0x000000FF00000000ULL) | ((value << 24) & 0x00FF000000000000ULL) |
           ((value << 40) & 0xFF00000000000000ULL);
#endif
}
}  // namespace
