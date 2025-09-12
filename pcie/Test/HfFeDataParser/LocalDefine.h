#pragma once
#include <cstdint>

struct _HF_ST_LINEHEAD_V15
{
    uint8_t rsvd8[47];

    uint8_t rsvd7 : 7;
    uint8_t sof : 1;

    uint16_t linlen;

    uint8_t pw_gap : 1;
    uint8_t dummy_line : 1;
    uint8_t freq_comp_param : 2;
    uint8_t img_mode : 4;

    uint8_t rsvd6 : 3;
    uint8_t depthid : 5;

    uint8_t rsvd5 : 2;
    uint8_t eof : 1;
    uint8_t cfm_ensember : 5;

    uint8_t rsvd4;
    uint8_t beamno;

    uint8_t rsvd3 : 1;
    uint8_t tfocus : 5;
    uint8_t bi_plane_mode : 2;

    uint8_t rsvd2 : 1;
    uint8_t ext_img_mode : 1;
    uint8_t spacial_comp : 1;
    uint8_t color_line_density : 2;
    uint8_t freq_comp : 1;
    uint8_t b_line_density : 2;

    uint8_t scanid;

    uint8_t rsvd : 7;
    uint8_t angle_indexL : 1;

    uint8_t angle_indexH;

    uint32_t tag;

    uint16_t getAngle()
    {
        return (uint16_t(angle_indexH) << 1) | angle_indexL;
    }
};

static_assert(sizeof(_HF_ST_LINEHEAD_V15) == 64, "size mismatch, check padding!");
typedef _HF_ST_LINEHEAD_V15 HFStLineHeader, *PHFStLineHeader;
