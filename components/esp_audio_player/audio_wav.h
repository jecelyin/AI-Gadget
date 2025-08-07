#pragma once

#include <stdio.h>
#include "audio_log.h"
#include "audio_decode_types.h"

typedef struct {
    // The "RIFF" chunk descriptor
    uint8_t ChunkID[4];
    int32_t ChunkSize;
    uint8_t Format[4];
    // The "fmt" sub-chunk
    uint8_t Subchunk1ID[4];
    int32_t Subchunk1Size;
    int16_t AudioFormat;
    int16_t NumChannels;
    int32_t SampleRate;
    int32_t ByteRate;
    int16_t BlockAlign;
    int16_t BitsPerSample;
} wav_header_t;

typedef struct {
    // The "data" sub-chunk
    uint8_t SubchunkID[4];
    int32_t SubchunkSize;
} wav_subchunk_header_t;

typedef struct {
    wav_header_t header;
    long data_start_pos;  // data 子块起始位置
    uint32_t data_size;   // data 子块大小
    uint32_t bytes_per_second; // 每秒字节数（缓存优化）
} wav_instance;

bool is_wav(FILE *fp, wav_instance *pInstance);
DECODE_STATUS decode_wav(FILE *fp, decode_data *pData, wav_instance *pInstance);

bool wav_seek(FILE *fp, wav_instance *pInstance, float target_time);
float get_wav_duration(wav_instance *pInstance);
float get_wav_current_position(FILE *fp, wav_instance *pInstance);
