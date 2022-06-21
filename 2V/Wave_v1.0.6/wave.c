/*
	author: Mario Friande
	number: 43785
	program: wave
	references: https://gist.github.com/Jon-Schneider/8b7c53d27a7a13346a643dac9c19d34f, 
		    https://www.youtube.com/watch?v=GExnnTaBELk,
		    https://www.youtube.com/watch?v=udbA7u1zYfc,
		    https://www.youtube.com/watch?v=e-srF6c3TJ8,
		    https://www.youtube.com/watch?v=P-fWNCF7Wx8,
		    https://embetronicx.com/tutorials/p_language/c/little-endian-and-big-endian/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include "list.h"
#include "wave.h"

#define BYTE_SIZE CHAR_BIT
// number of bytes that are static in wave header
#define STATIC_CONTENT 44
// MACRO TO CONVERT ONE ENDIAN TO ANOTHER	
#define convert(value) ((0x000000ff & value) << 24) | ((0x0000ff00 & value) << 8) | ((0x00ff0000 & value) >> 8) | ((0xff000000 & value) >> 24)

//a) Defina o tipo de dados Wave e programe as funções indicadas acima, de acordo com as definições dadas.
//	Baseie-se no formato canónico da norma WAVE definido neste documento: WAVE PCM soundfile format.

// criar um objecto Wave
Wave *wave_new() {
	Wave *wave = malloc(sizeof *wave);
	if(NULL == wave) {
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}
	//wave->data = NULL;
	wave->data = list_create();
	wave->data_chunk_size = 0;
	return wave;
}		

// eliminar um objecto Wave vazio
void wave_destroy(Wave *wave) {
 	// free pointer to link (first node queue)
	data_block_leak_list(wave);
	// release wave
	free(wave->data); // to release memory from sentinel Node
	free(wave);
}

// Definir o numero de bits por amostra 
void wave_set_bits_per_sample(Wave *wave, int bits_per_sample) {
	wave->bits_per_sample = bits_per_sample;
}

// Obter o número de bits por amostra;
int wave_get_bits_per_sample(Wave *wave){
	return wave->bits_per_sample;
}

// Definir o número de canais;
void wave_set_number_of_channels(Wave *wave, int number_of_channels) {
	wave->num_channels = number_of_channels;
}

// Obter o número de canais;
int wave_get_number_of_channels(Wave *wave) {
	return wave->num_channels;
}

// Definir o ritmo de amostragem;
void wave_set_sample_rate(Wave *wave, int sample_rate) {
	wave->sample_rate = sample_rate;
}
// Obter o ritmo de amostragem;
int wave_get_sample_rate(Wave *wave) {
	return wave->sample_rate;
}
/*
	Acrescentar no final da informação áudio existente no objeto wave, e copiando de buffer, 
	a sequência de frames com a dimensão frame_count. Uma frame é um conjunto de amostras, 
	uma por cada canal. A função devolve o número de frames efetivamente copiadas.
*/
size_t wave_append_samples(Wave *wave, uint8_t *buffer, size_t frame_count) {
	size_t frame_size = wave_get_bits_per_sample(wave) * wave->num_channels / BYTE_SIZE; // eg: mono = bits per sample * 1 || stereo = bits per sample * 2
	Data_Block *data = wave_create_data_block(frame_size, frame_count);
	// todo 
	//printf("frame size = %ld\n", frame_size);
	memcpy(data->bytes, buffer, frame_size * frame_count);	
	
	list_insert_rear(wave->data, data); 
	wave->data_chunk_size += frame_count * frame_size;
	//printf("chunk_size = %d\n", wave->data_chunk_size);
	return frame_count;
}

Data_Block *wave_create_data_block(size_t frame_size, size_t frame_count) {
	Data_Block *data = malloc(sizeof *data);
	if(NULL == data) {
		fprintf(stderr, "Out of memory\n");
		exit(-1);
	}	
	data->bytes = malloc(frame_size * frame_count);
	if(NULL == data->bytes) {
		fprintf(stderr, "Out of memory\n");
		exit(-1);
	}	
	data->frame_count = frame_count;
	data->frame_size = frame_size;
	return data;
}

// Criar um ficheiro em formato WAVE com o nome filename e com o conteúdo indicado por wave.
int wave_store(Wave *wave, char *filename) {
	// todo
	char buffer[STATIC_CONTENT] = {0}; // buffer to write first 44 bytes of wave header
	FILE *out = fopen(filename, "wb");
	if(NULL == out) {
		fprintf(stderr, "Error opening \"%s\n\"", filename);
		return -1;
	}
	
	// writing to file wave header static content
	write_wave_header_info(wave, buffer);
	int res = fwrite(buffer, STATIC_CONTENT, 1, out); // The fwrite() function returns the number of members successfully written 
	if(res != 1) {
		fprintf(stderr, "Couldnn't write file!\n");
		return -1;
	} 
	res = list_foreach_write(wave->data, write_data_block_to_file, out);
	if(res != 1) { // before was res != 0
		fprintf(stderr, "Couldnnn't write file!\n");
		return -1;
	} 
	res = fclose(out);
	if(res != 0) {
		fprintf(stderr, "Couldn't close file!\n");
		return -1;
	} 
	return 0; // code 0 success / code -1 something went wrong
}

// retrieves all data from nodes and write to file

int write_data_block_to_file(void *data_block, FILE *file) {
	return fwrite(((Data_Block *)data_block)->bytes, ((Data_Block *)data_block)->frame_count * ((Data_Block *)data_block)->frame_size, 1, file); // rever esta parte
}

// writes static wave header content 
void write_wave_header_info(Wave *wave, char *buffer) {
	int32_t temp = (int32_t)(wave->data_chunk_size + STATIC_CONTENT - BYTE_SIZE); 
	memcpy(buffer, RIFF_HEADER, 4); 	// "RIFF"
	memcpy(buffer+4, &temp, 4);		// Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
	memcpy(buffer+8, WAVE_HEADER, 4);	// "WAVE"
	memcpy(buffer+12, FMT_HEADER, 4);	// "fmt "
	buffer[16] = (int32_t)CHUNK_SIZE_PCM;	// should be 16 for PCM
	buffer[20] = (int16_t)AUDIO_FORMAT_PCM; // Should be 1 for PCM.
	buffer[22] = (int16_t)wave->num_channels;	// num_channels
	memcpy(buffer+24, &(wave->sample_rate), 4);	// sample_rate
	temp = (int32_t)(wave->sample_rate * wave->num_channels * wave->bits_per_sample / BYTE_SIZE);
	memcpy(buffer+28, &temp, 4); // byte_rate
	buffer[32] = (int16_t)(wave->num_channels * wave->bits_per_sample / BYTE_SIZE); // sample alignment
	buffer[34] = (int16_t)(wave->bits_per_sample); // number of bits per sample;
	memcpy(buffer+36, DATA_HEADER, 4);	// "data"
	memcpy(buffer+40, &(wave->data_chunk_size), 4); // Number of bytes in data. Number of samples * num_channels * sample byte size
}
// iterates through list and deletes each node that contains audio data freeing memory to prevent leaks
void data_block_leak_list(Wave *wave) {
	list_foreach(wave->data, data_block_delete);
	list_delete(wave->data);
}
// delete a data_block freeing pointer bytes and struct data_block
void data_block_delete(void *data_block) {
	free(((Data_Block *)data_block)->bytes);
	free(data_block);
}
