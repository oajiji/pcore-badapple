#ifndef WAV_H
#define WAV_H

#include <stdint.h>

/// String byte defines.
/**
 * The struct contains some unsigned int32 constants which 
 * is equal to a 4-byte string.
 */
static const uint32_t RIFF = 0x46464952;
static const uint32_t WAVE = 0x45564157;
static const uint32_t FMT_ = 0x20746d66;
static const uint32_t FACT = 0x74636166;
static const uint32_t DATA = 0x61746164;

/// RIFF WAVE Chunk (12 bits)
struct RIFF_CHUNK {
    uint32_t fccID;       ///< 必须是 "RIFF"
    uint32_t dwSize;      ///< 文件总的字节数减去8 
    uint32_t fccType;     ///< 必须是 "WAVE"
};

/// Format Chunk (24+2 bits)
struct FORMAT_CHUNK {
    uint32_t fccID;           ///< 必须是 "fmt "
    uint32_t dwSize;          ///< 数值为16或18，18则最后又附加信息
    uint16_t wFormatTag;      ///< 编码方式，一般为 0x1（无压缩）
                            ///< 0x6表示A律压缩，0x7表示u律压缩
    uint16_t wChannels;       ///< 声道数目，1--单声道；2--双声道
    uint32_t dwSamplesPerSec; ///< 采样频率
    uint32_t dwAvgBytePerSec; ///< 每秒所需字节数
    uint16_t wBlockAlign;     ///< 数据块对齐单位(每个采样需要的字节数) 
    uint16_t uiBitsPerSample; ///< 每个采样需要的bit数
    uint16_t wExtraBits;      ///< 附加信息（可选，通过Size来判断有无）
};

/// Fact Chunk (12 bits)
struct FACT_CHUNK {
    uint32_t fccID;   ///< 必须是 "fact"
    uint32_t dwSize;
    uint32_t data;
};

/// Data Chunk (8 bits)
struct DATA_CHUNK {
    uint32_t fccID;   ///< 必须是 "data"
    uint32_t dwSize;  ///< 音频数据的样本数量
};

#endif  // WAV_H