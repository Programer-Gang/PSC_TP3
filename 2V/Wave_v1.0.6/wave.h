#ifndef WAVE_H
#define WAVE_H

#include "list.h"

#define RIFF_HEADER "RIFF"
#define WAVE_HEADER "WAVE"
#define FMT_HEADER "fmt "
#define CHUNK_SIZE_PCM 16
#define AUDIO_FORMAT_PCM 1
#define DATA_HEADER "data"

typedef struct data_block {
	size_t frame_count;
	size_t frame_size;
	size_t *bytes;
} Data_Block;

typedef struct wav_header {
    	short num_channels;
    	int sample_rate;
    	short bits_per_sample; 
	int data_chunk_size; // Number of bytes in data. Number of samples * num_channels * sample byte size
	Node *data;	// pointer to list containing data audio information
	//Wave *next, *prev; // ## alterado 13/06/2022 ## pointer to next and prev Wave elements -> important when is necessary list all Wave captures
} Wave;

// criar um objecto wave
Wave *wave_new();
// criar wave struct
Wave *wave_create();
// eliminar um objecto Wave vazio
void wave_destroy(Wave *wave);
// Definir o numero de bits por amostra 
void wave_set_bits_per_sample(Wave *wave, int bits_per_sample);
// Obter o número de bits por amostra;
int wave_get_bits_per_sample(Wave *wave);
// Definir o número de canais;
void wave_set_number_of_channels(Wave *wave, int number_of_channels);
// Obter o número de canais;
int wave_get_number_of_channels(Wave *wave);
// Definir o ritmo de amostragem;
void wave_set_sample_rate(Wave *wave, int sample_rate);
// Obter o ritmo de amostragem;
int wave_get_sample_rate(Wave *wave);
/*
	Acrescentar no final da informação áudio existente no objeto wave, e copiando de buffer, 
	a sequência de frames com a dimensão frame_count. Uma frame é um conjunto de amostras, 
	uma por cada canal. A função devolve o número de frames efetivamente copiadas.
*/
size_t wave_append_samples(Wave *wave, uint8_t *buffer, size_t frame_count);

Data_Block *wave_create_data_block();
// Criar um ficheiro em formato WAVE com o nome filename e com o conteúdo indicado por wave.
int wave_store(Wave *wave, char *filename);
// Function to retrieve all audio data from nodes and write to file
int write_data_block_to_file(void *data_block, FILE *file);
// writes static wave header content 
void write_wave_header_info(Wave *wave, char *buffer);
// iterates through list and deletes each node freeing memory to prevent leaks
void data_block_leak_list(Wave *wave);
// delete a data_block freeing pointer bytes and struct data_block
void data_block_delete(void *data_block);
#endif
