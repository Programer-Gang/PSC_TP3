#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <alsa/asoundlib.h>
#include "wave.h"
#include "../utils/dlist.h"

#define DATA_OFFSET 44
#define SOUND_DEVICE "default"

Wave *wave_create()
{
    Wave *wave = malloc(sizeof *wave);
    if (wave == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        exit(-1);
    }

    Node *node_data_list = list_create();

    wave->chunk_id[0] = "RIFF";
    wave->format[0] = "WAVE";
    wave->sub_chunk_1_id[0] = "fmt ";
    wave->sub_chunk_1_size = 16;
    wave->audio_format = 1;
    wave->num_channels = 1;
    wave->sample_rate = 44100;
    wave->block_align = 2; // NumChannels * BitsPerSample/8
    wave->bits_per_sample = 16;
    wave->sub_chunk_2_id[0] = "data";
    wave->sub_chunk_2_size = 0;
    // Subchunk2Size == NumSamples * NumChannels * BitsPerSample/8
    wave->chunk_size = 36;
    int x = wave->sample_rate;
    int y = wave->bits_per_sample;
    int z = wave->num_channels;
    wave->byte_rate = (x * y * z) / 8;
    wave->wave_data_list = node_data_list;
}

int wave_store(Wave *wave, char *filename)
{

    //VERIFY IF num_channels, sample_rate and bits_per_sample are different from 0
    hasChanged(wave->num_channels);
    hasChanged(wave->sample_rate);
    hasChanged(wave->bits_per_sample);

    FILE *ptr = fopen(filename, "wb");
    if (ptr == NULL)
    {
        printf("Error opening file\n");
        exit(-1);
    }
    wave->file = ptr;

    const unsigned int bytes_per_sample = wave_get_bits_per_sample(wave) / 8;
    wave->byte_rate = wave_get_sample_rate(wave) * bytes_per_sample;
    wave->block_align = wave_get_number_of_channels(wave) * bytes_per_sample;
    //wave->bits_per_sample ALREADY DEFINED
    wave->sub_chunk_2_id[0] = "data";
    //TODO how do you know sample_count?
    wave->sub_chunk_2_size = ?sample_count? * wave->block_align;

    // Escrita do Header no ficheiro
    fwrite((void *)&wave->chunk_id, DATA_OFFSET, 1, ptr);

    const unsigned int frame_size = bytes_per_sample * wave->num_channels;
    unsigned int frame_count = 0;

    Node *list = wave->wave_data_list;
    // Escrita dos blocos de frames no ficheiro
    for (Node *p = list->next; p != list; p = p->next)
    {
        int result = fwrite(p->data, frame_size, 1, ptr);
        if (result != 0)
        {
            frame_count += result;
        }
        else
        {
            printf("Frames Written before error %d\n", frame_count);
            printf("Error writing on file\n", filename);
            exit(1);
        }
    }

    // Atualizar o tamanho do ficheiro no header depois da escrita

    return frame_count;
}

Wave *wave_load(const char *filename)
{
    FILE *ptr = fopen(filename, "rb");
    if (ptr == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    Wave *wave = malloc(sizeof *wave);
    if (wave == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        exit(-1);
    }

    wave->file = ptr;
    fread((void *)&wave->chunk_id, 1, sizeof *wave - sizeof wave->file, ptr);

    return wave;
}

void wave_destroy(Wave *wave)
{
    fclose(wave->file);

    Node *list = wave->wave_data_list;
    for (Node *p = list->previous; list->previous != list; p = p->previous)
    {
        p->prev->next = p->next;
        p->next->prev = p->prev;
        free(p->data);
        free(p);
    }
    free(wave);
}

void wave_set_bits_per_sample(Wave *wave, int bits_per_sample)
{
    wave->bits_per_sample = bits_per_sample;
}

int wave_get_bits_per_sample(Wave *wave)
{
    return wave->bits_per_sample;
}

void wave_set_number_of_channels(Wave *wave, int number_of_channels)
{
    wave->num_channels = number_of_channels;
}

int wave_get_number_of_channels(Wave *wave)
{
    return wave->num_channels;
}

void wave_set_sample_rate(Wave *wave, int sample_rate)
{
    wave->sample_rate = sample_rate;
}

int wave_get_sample_rate(Wave *wave)
{
    return wave->sample_rate;
}

size_t wave_append_samples(Wave *wave, uint8_t *buffer, size_t frame_count)
{
    const unsigned int bytes_per_sample = wave_get_bits_per_sample(wave) / 8;
    const unsigned int frame_size = bytes_per_sample * wave->num_channels;

    Node *list = wave->wave_data_list;
    unsigned int i = 0;
    for (unsigned int frame_offset = 0; i < frame_count; i++, frame_offset += frame_size)
    {
        uint8_t *data = malloc(frame_size);
        if (NULL == data)
        {
            fprintf(stderr, "Out of memory\n");
            exit(-1);
        }
        memcpy(data, buffer[frame_offset], frame_size);
        list_insert_rear(list, data);
    }
    return i;
}

size_t wave_get_samples(Wave *wave, size_t frame_index,
                        char *buffer, size_t frame_count)
{
    const unsigned int bytes_per_sample = wave_get_bits_per_sample(wave) / 8;
    const unsigned int frame_size = bytes_per_sample * wave->num_channels;
    const int result = fseek(wave->file, DATA_OFFSET + (frame_index * frame_size), SEEK_SET);
    if (result != 0)
    {
        printf("fseek error\n");
        exit(1);
    }
    return fread(buffer, frame_size, frame_count, wave->file);
}

void wave_play(Wave *wave)
{
    snd_pcm_t *handle = NULL;
    int result = snd_pcm_open(&handle, SOUND_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
    if (result < 0)
    {
        printf("snd_pcm_open(&handle, %s, SND_PCM_STREAM_PLAYBACK, 0): %s\n",
               SOUND_DEVICE, snd_strerror(result));
        exit(EXIT_FAILURE);
    }

    snd_config_update_free_global();

    result = snd_pcm_set_params(handle,
                                SND_PCM_FORMAT_S16_LE,
                                SND_PCM_ACCESS_RW_INTERLEAVED,
                                wave_get_number_of_channels(wave),
                                wave_get_sample_rate(wave),
                                1,
                                500000);
    if (result < 0)
    {
        fprintf(stderr, "Playback open error: %s\n", snd_strerror(result));
        exit(EXIT_FAILURE);
    }

    const snd_pcm_sframes_t period_size = 64;
    int frame_size = snd_pcm_frames_to_bytes(handle, 1);

    uint8_t buffer[period_size * frame_size];
    size_t frame_index = 0;

    size_t read_frames = wave_get_samples(wave, frame_index, (char *)buffer, period_size);

    while (read_frames > 0)
    {
        snd_pcm_sframes_t wrote_frames = snd_pcm_writei(handle, buffer, read_frames);
        if (wrote_frames < 0)
            wrote_frames = snd_pcm_recover(handle, wrote_frames, 0);
        if (wrote_frames < 0)
        {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(wrote_frames));
            break;
        }

        if (wrote_frames < read_frames)
            fprintf(stderr, "Short write (expected %li, wrote %li)\n",
                    read_frames, wrote_frames);

        frame_index += period_size;
        read_frames = wave_get_samples(wave, frame_index, (char *)buffer, period_size);
    }
    /* pass the remaining samples, otherwise they're dropped in close */
    result = snd_pcm_drain(handle);
    if (result < 0)
        printf("snd_pcm_drain failed: %s\n", snd_strerror(result));

    snd_pcm_close(handle);
    snd_config_update_free_global();
}