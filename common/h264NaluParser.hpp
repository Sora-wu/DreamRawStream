//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <cstdint>
#include <cstring>

enum class H264NalType {
    H264NT_NAL = 0,
    H264NT_SLICE,
    H264NT_SLICE_DPA,
    H264NT_SLICE_DPB,
    H264NT_SLICE_DPC,
    H264NT_SLICE_IDR,
    H264NT_SEI,
    H264NT_SPS,
    H264NT_PPS,
};

struct H264NaluHeader {
    H264NalType type = H264NalType::H264NT_NAL;
    char* data = nullptr;       // 去掉startcode的数据
    uint32_t size = 0;          // 去掉startcode的长度
};

// 从 H.264 Annex B 格式的数据中解析出一个 NALU
// 返回 NALU 信息（不包括 startcode）
inline H264NaluHeader parseH264Nalu(char* h264, uint32_t len) {
    H264NaluHeader header{};
    if (!h264 || len < 4) {
        return header;
    }

    char* start = h264;
    char* end = h264 + len;
    char* naluStart = nullptr;
    // 1. 查找第一个起始码
    for (char* p = start; p <= end - 3; ++p) {
        if (p[0] == 0x00 && p[1] == 0x00) {
            if (p[2] == 0x01) {
                naluStart = p + 3;      // 3字节起始码
                break;
            }
            if (p[2] == 0x00 && (p + 3) < end && p[3] == 0x01) {
                naluStart = p + 4;      // 4字节起始码
                break;
            }
        }
    }

    if (!naluStart || naluStart >= end) {
        return header;
    }

    char* dataStart = naluStart;
    char* nextStart = nullptr;
    // 2. 查找下一个起始码，确定当前 NALU 结束位置
    for (char* p = naluStart; p <= end - 3; ++p) {
        if (p[0] == 0x00 && p[1] == 0x00) {
            if (p[2] == 0x01) {
                nextStart = p;
                break;
            }
            if (p[2] == 0x00 && (p + 3) < end && p[3] == 0x01) {
                nextStart = p;
                break;
            }
        }
    }

    header.size = nextStart ? static_cast<uint32_t>(nextStart - dataStart) : static_cast<uint32_t>(end - dataStart);
    if (header.size == 0) {
        return header;
    }

    // 3. 解析 NALU 类型（低5位）
    const uint8_t nalHeaderByte = static_cast<uint8_t>(dataStart[0]);
    const int nalUnitType = nalHeaderByte & 0x1F;
    switch (nalUnitType) {
        case 1:  header.type = H264NalType::H264NT_SLICE;      break;
        case 2:  header.type = H264NalType::H264NT_SLICE_DPA;  break;
        case 3:  header.type = H264NalType::H264NT_SLICE_DPB;  break;
        case 4:  header.type = H264NalType::H264NT_SLICE_DPC;  break;
        case 5:  header.type = H264NalType::H264NT_SLICE_IDR;  break;
        case 6:  header.type = H264NalType::H264NT_SEI;        break;
        case 7:  header.type = H264NalType::H264NT_SPS;        break;
        case 8:  header.type = H264NalType::H264NT_PPS;        break;
        default: header.type = H264NalType::H264NT_NAL;        break;
    }

    header.data = dataStart;
    return header;
}
