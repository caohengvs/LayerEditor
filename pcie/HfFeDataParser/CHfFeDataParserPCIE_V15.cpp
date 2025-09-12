#include "pch.h"
#include "CHfFeDataParserPCIE_V15.h"
const size_t LINEHEADER_SIZE = 64;
CHfFeDataParserPCIE_V15::CHfFeDataParserPCIE_V15()
{
}

std::vector<CHfFeDataParserPCIE_V15::StLineInfo> CHfFeDataParserPCIE_V15::GetLineData()
{
    m_lines.clear();

    size_t offset = 0;
    while (offset + LINEHEADER_SIZE <= m_rawSize)
    {
        auto* pHdr = reinterpret_cast<PHFStLineHeader>(m_raw + offset);
        if (pHdr->tag != 0x80008000)
            continue;

        size_t rfSize = static_cast<size_t>(pHdr->linlen) * 16;
        size_t nextOffset = offset + LINEHEADER_SIZE + rfSize;

        if (nextOffset > m_rawSize)
            break; 

        m_lines.push_back({offset, offset + LINEHEADER_SIZE, rfSize});
        offset = nextOffset;
    }
    return m_lines;
}
