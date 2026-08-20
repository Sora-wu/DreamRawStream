//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <camera/fdkAACEncoder.h>

#include <print>

FdkAACEncoder::FdkAACEncoder(uint32_t sampleRate, uint32_t channels, uint32_t bytesPerSample)
    : sampleRate_(sampleRate), channels_(channels), bytesPerSample_(bytesPerSample) {
    init();
}

FdkAACEncoder::~FdkAACEncoder() {
    if (aacHandle_) {
        aacEncClose(&aacHandle_);
        aacHandle_ = nullptr;
    }
}

std::span<char> FdkAACEncoder::encode(std::span<char> inPcmBuffer) {
    if (!aacHandle_ || inPcmBuffer.empty()) {
        return {};
    }

    AACENC_BufDesc inBufDesc{};
    AACENC_BufDesc outBufDesc{};
    AACENC_InArgs inArgs{};
    AACENC_OutArgs outArgs{};

    int inIdentifier = IN_AUDIO_DATA;
    int inElemSize = bytesPerSample_;
    void* inPtr = inPcmBuffer.data();
    int inSize = (int)(inPcmBuffer.size());

    inBufDesc.numBufs = 1;
    inBufDesc.bufs = &inPtr;
    inBufDesc.bufferIdentifiers = &inIdentifier;
    inBufDesc.bufSizes = &inSize;
    inBufDesc.bufElSizes = &inElemSize;

    // 输入的 16bit 样本总数 (字节数 / 2)
    inArgs.numInSamples = inSize / 2;

    int outIdentifier = OUT_BITSTREAM_DATA;
    int outElemSize = 1;
    void* outPtr = outBuffer_.data();
    int outSize = (int)(outBuffer_.size());

    outBufDesc.numBufs = 1;
    outBufDesc.bufs = &outPtr;
    outBufDesc.bufferIdentifiers = &outIdentifier;
    outBufDesc.bufSizes = &outSize;
    outBufDesc.bufElSizes = &outElemSize;

    AACENC_ERROR err = aacEncEncode(aacHandle_, &inBufDesc, &outBufDesc, &inArgs, &outArgs);
    if (err != AACENC_OK) {
        std::println(stderr, "AAC encoding failed with error code: {:04x}", (uint32_t)err);
        return {};
    }

    if (outArgs.numOutBytes > 0) {
        return {outBuffer_.data(), (uint32_t)(outArgs.numOutBytes)};
    }

    // 有些情况下（如送入数据不足一帧），编码器会缓冲而不输出数据，此时返回空
    return {};
}

void FdkAACEncoder::init() {
    AACENC_ERROR err;

    err = aacEncOpen(&aacHandle_, 0, channels_);
    if (err != AACENC_OK) {
        std::println("Failed to open FDK AAC encoder");
        exit(2);
    }

    // 设置编码器参数: AOT_AAC_LC = 2 (最常用的 Low Complexity 模式)
    err = aacEncoder_SetParam(aacHandle_, AACENC_AOT, 2);
    if (err != AACENC_OK) {
        std::println(stderr,"Failed to set AACENC_AOT");
        exit(2);
    }

    err = aacEncoder_SetParam(aacHandle_, AACENC_SAMPLERATE, sampleRate_);
    if (err != AACENC_OK) {
        std::println(stderr,"Failed to set AACENC_SAMPLERATE");
        exit(2);
    }

    // 根据通道数设置 Channel Mode
    CHANNEL_MODE mode = channels_ == 1 ? MODE_1 : MODE_2;
    err = aacEncoder_SetParam(aacHandle_, AACENC_CHANNELMODE, mode);
    if (err != AACENC_OK) {
        std::println(stderr,"Failed to set AACENC_CHANNELMODE");
        exit(2);
    }

    const uint32_t bitRate = sampleRate_ * bytesPerSample_ * channels_;
    err = aacEncoder_SetParam(aacHandle_, AACENC_BITRATE, bitRate);
    if (err != AACENC_OK) {
        std::println(stderr,"Failed to set AACENC_BITRATE");
        exit(2);
    }

    // 2 表示携带 ADTS 头部 0 表示Raw
    err = aacEncoder_SetParam(aacHandle_, AACENC_TRANSMUX, 2);
    if (err != AACENC_OK) {
        std::println(stderr,"Failed to set AACENC_TRANSMUX");
        exit(2);
    }

    // 提交配置，初始化编码器实例
    err = aacEncEncode(aacHandle_, nullptr, nullptr, nullptr, nullptr);
    if (err != AACENC_OK) {
        std::println("Failed to initialize FDK AAC encoder instance");
        exit(2);
    }

    AACENC_InfoStruct info{};
    err = aacEncInfo(aacHandle_, &info);
    if (err != AACENC_OK) {
        std::println("Failed to get FDK AAC encoder info");
        exit(2);
    }

    outBuffer_.resize(info.maxOutBufBytes);
}
