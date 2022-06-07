#ifndef WAVE_H
#define WAVE_H

#include <stdlib.h>
#include <stdint.h>

typedef struct wave
{
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
    void *wave_data_list;
} Wave;

typedef struct databuffer
{
    unsigned int data_frame_count;
    void *heap_data;
} DataBuffer;

Wave *wave_create();

Wave *wave_load(const char *filename);

void wave_destroy(Wave *wave);

void wave_set_bits_per_sample(Wave *wave, int bits_per_sample);

int wave_get_bits_per_sample(Wave *wave);

void wave_set_number_of_channels(Wave *wave, int number_of_channels);

int wave_get_number_of_channels(Wave *wave);

void wave_set_sample_rate(Wave *wave, int sample_rate);

int wave_get_sample_rate(Wave *wave);

size_t wave_get_samples(Wave *wave, size_t frame_index,
                        char *buffer, size_t frame_count);

size_t wave_append_samples(Wave *wave, uint8_t *buffer, size_t frame_count);

int wave_store(Wave *wave, char *filename);

void wave_play(Wave *wave);

#endif