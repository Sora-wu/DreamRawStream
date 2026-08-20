//
// Author: sora
// Mail: sora-wu@foxmail.com
//
// AAC ADTS 头部解析工具
// 从 rtmpPublisher.cpp 中提取，供多个推流器复用
//

#pragma once

#include <cstdint>

struct AacAdtsInfo {
    uint32_t profileId = 0;       // 0=Main, 1=LC, 2=SSR
    uint32_t sampleRateId = 0;    // 索引: 3=48000, 4=44100, 5=32000 等
    uint32_t channelConfig = 0;   // 1=单声道, 2=双声道 等
    uint32_t sampleRate = 0;      // 实际采样率 (Hz)
    bool valid = false;
};

// 根据 ADTS 固定头解析 AAC 音频参数
// ADTS 固定头结构 (共28位):
//   syncword(12):        固定为 0xFFF
//   ID/MPEG版本(1):      0=MPEG-4, 1=MPEG-2
//   layer(2):            固定为 00
//   protection_absent(1): 1=无CRC, 0=有CRC (头会多2字节)
//   profile(2):          00=Main, 01=LC, 10=SSR
//   sampling_frequency_index(4): 3=48000, 4=44100, 5=32000 等
//   private_bit(1):      通常为 0
//   channel_configuration(3): 1=单声道, 2=双声道, 3=三声道 等
//   original_copy(1):    版权标识
//   home(1):             家庭标识
inline AacAdtsInfo parseAacAdts(const char* data, uint32_t len) {
    AacAdtsInfo info{};
    if (!data || len < 7) {
        return info;
    }

    // 验证同步字（前12位应为 0xFFF）
    if ((static_cast<uint8_t>(data[0]) != 0xFF) || ((static_cast<uint8_t>(data[1]) & 0xF0) != 0xF0)) {
        return info;
    }

    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    info.profileId = (p[2] >> 6) & 0x03;
    info.sampleRateId = (p[2] >> 2) & 0x0F;
    info.channelConfig = ((p[2] & 0x01) << 2) | ((p[3] >> 6) & 0x03);

    // 将采样率索引转换为实际采样率
    static const uint32_t sampleRateTable[] = {
        96000, 88200, 64000, 48000, 44100, 32000, 24000,
        22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0
    };
    if (info.sampleRateId < 13) {
        info.sampleRate = sampleRateTable[info.sampleRateId];
    }

    info.valid = true;
    return info;
}

// 生成 AAC AudioSpecificConfig (2字节)
// 用于 SDP fmtp config 参数
inline uint16_t makeAudioSpecificConfig(uint32_t profileId, uint32_t sampleRateId, uint32_t channelConfig) {
    uint16_t config = 0;
    config |= (profileId << 11);          // 5 bits: audioObjectType
    config |= (sampleRateId << 7);        // 4 bits: samplingFrequencyIndex
    config |= (channelConfig << 3);       // 4 bits: channelConfiguration
    // 低3位为填充位 (0)
    return config;
}
