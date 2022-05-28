#ifndef WAVE_H
#define WAVE_H

#include <stdlib.h>

typedef struct wave {
    FILE *file;
    unsigned char chunk_id[4];
    unsigned int chunk_size;
    unsigned char format[4];
    unsigned char sub_chunk_1_id[4];
    unsigned int sub_chunk_1_size;
    unsigned short audio_format;
    unsigned short num_channels;
    unsigned int sample_rate;
    unsigned int byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
    unsigned char sub_chunk_2_id[4];
    unsigned int sub_chunk_2_size;
} Wave;

Wave *wave_load(const char *filename);

void wave_destroy(Wave *wave);

int wave_get_bits_per_sample(Wave *wave);

int wave_get_number_of_channels(Wave *wave);

int wave_get_sample_rate(Wave *wave);

size_t wave_get_samples(Wave *wave, size_t frame_index,
                        char *buffer, size_t frame_count);

void wave_play(Wave *wave);

#endif