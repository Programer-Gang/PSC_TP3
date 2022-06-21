/*
	author: Mario Friande
	number: 43785
	program: capture_program
	references: 
*/

/*
	Construa um programa de captura de informação áudio para ficheiros em formato WAVE. O programa
	executa um ciclo permanente de aceitação e execução de comandos. Os comandos a implementar são: start,
	stop, list, save e exit. O comando start desencadeia uma captura de áudio; o comando stop termina a
	captura que está a decorrer; o comando list mostra as capturas realizadas; o comando save guarda em
	ficheiro WAVE a captura indicada em argumento; o comando exit termina o programa.
*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <threads.h>
#include <alsa/asoundlib.h>
#include <signal.h>
/*-----------------------------------------------------------------------------
	A fila de ficheiros wav é representada por um elemento Wave,
	utilizado como sentinela
 */

#include "wave.h"
#include "list.h"

volatile int running;
thrd_t thread;
Node *wave_list;
static const snd_pcm_sframes_t period_size = 1024;
	
/*-----------------------------------------------------------------------------------------------------------
	Function to start wav capture
*/
// uma funcao para fazer a captura

int wave_capture() {
	Wave *wave = wave_new();
	if (wave == NULL) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	wave_set_number_of_channels(wave, 1);
	wave_set_sample_rate(wave, 44100);
	wave_set_bits_per_sample(wave, 16);

	snd_pcm_t *handle;
	int result = snd_pcm_open (&handle, "default", SND_PCM_STREAM_CAPTURE, 0);
	if (result < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n",
			 "default",
			 snd_strerror (result));
			 return -1;
	}
	result = snd_pcm_set_params(handle,
					  SND_PCM_FORMAT_S16_LE,
					  SND_PCM_ACCESS_RW_INTERLEAVED,
					  wave_get_number_of_channels(wave),
					  wave_get_sample_rate(wave),
					  1,
					  500000);   /* 0.5 sec */
	if (result < 0) {
		fprintf(stderr, "snd_pcm_set_params: %s\n",
			snd_strerror(result));
			return -1;
    	}
	result = snd_pcm_prepare(handle);
	if (result < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror(result));
		return -1;
	}

	int frame_size = snd_pcm_frames_to_bytes(handle, 1);
	uint8_t buffer[period_size * frame_size];
	snd_pcm_sframes_t read_frames;
	int ten_seconds = 10 * wave_get_sample_rate(wave);
	for (int frame_index = 0; frame_index < ten_seconds; frame_index += read_frames) {
		if(!running) 
			break;
		read_frames = snd_pcm_readi(handle, buffer, period_size);
		if (read_frames < 0) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
					snd_strerror(read_frames));
			return -1;
		}

		wave_append_samples(wave, buffer, read_frames);
		
		//print_samples(buffer, read_frames, wave_get_number_of_channels(wave));
	}

	snd_pcm_close (handle);
	// insert wave capture at the end of list 
	list_insert_rear(wave_list, wave);
	return 0; // if success returns value 0 / if something goes wrong returns -1
}

void start_capture() {
	running = 1;
	printf("Starting capture...\n");
	thrd_create(&thread, (thrd_start_t) wave_capture, NULL);
}

/*-----------------------------------------------------------------------------------------------------------
	Function to stop wav capture
*/
void stop_capture() {
	int result;
	running = 0;
	printf("Stopping capture...\n");
	thrd_join(thread, &result);
	printf("After th join!\n");
}

// uma funcao para terminar a captura ??


// Function to print wave captures information

void print_capture_info(void *wave_capture, int index) {
	printf("Wave capture nr: %d\n", index); // for now just printing index
}

/*-----------------------------------------------------------------------------------------------------------
	Function to list all wav captures
*/
void list_captures() {
	//todo só imprimir cada nó da lista list_foreach
	if(list_empty(wave_list)) {
		printf("No entries on wave capture list\n");
		return;
	}
	printf("#### Wave captures list ####\n");
	list_foreach_captures(wave_list, print_capture_info);
}

// Function to get wave capture according to index

int get_capture(void *wave_capture, int index, int match) {
	if(index == match) {
		return 1; // code 1 if found
	}		
	return 0; // code 0 if not found		
}

/*-----------------------------------------------------------------------------------------------------------
	Function to save wav capture to file
*/
void save_capture() {
	//todo // chamar o wave_store ?? duvida como se identifica cada wave ?? period como interfere na captura do som ?? mostrar captura parece ter filtro para voz fina
	char nr; // local variable stores nr of capture retrieved
	char name[100]; // to store filename 
	Wave *wave;
	if(!list_empty(wave_list)) {	
		printf("#### choose the number of one of the following captures to record to file ####\n");
		list_captures();
		printf("> ");
		nr = getchar() - '0';
	}
	else {
		printf("There are no entries on wave capture list, please make some captures first!\n");
		return;
	}
	wave = (Wave *)list_get_wave_capture(wave_list, get_capture, nr);
	printf("please introduce a filename\n> ");
	fgets(name, sizeof(name), stdin);
	printf("%s", name); // sanity check ## 
	//strcpy(name, "sample");
	int outcome = wave_store(wave, name);
	if(outcome != 0) {
		fprintf(stderr, "error: wave file couldn't be saved!\n");
		return;
	}
}

/*-----------------------------------------------------------------------------------------------------------
	Function to exit program

*/

void exit_program(char *unused){
	//todo // chamar o wave destroy
}

/*===========================================================================================================
* Commands
*/

typedef struct command {
	void (*f) (char *);
	char c;
	char *desc;
	struct command *next;
} Command;

static Command *commands = NULL;

void command_insert(char c, char *desc, void (*f) (char *)) {
	Command *new_command = malloc(sizeof(Command));
	if(NULL == new_command) {
		fprintf(stderr, "Out of memory!\n");
		exit(-1);
	}
	new_command->c = c;
	new_command->desc = strdup(desc);
	new_command->f = f;
	new_command->next = commands;
	commands = new_command;
}

void command_execute(char c, char *param) {
	for(Command *p = commands; p != NULL; p = p->next)
		if(p->c == c) {
			p->f(param);
			return;
		}
}

void command_list(char *unused) {
	for(Command *p = commands; p != NULL; p = p->next)
		printf("%c%s\n", p->c, p->desc);
}

int main() {
	char line[100];
	wave_list = list_create(); // initializing wave_list sentinel node
	command_insert('e', "\t - Exit", exit_program);
	command_insert('h', "\t - List all available commands", command_list);
	command_insert('r', "\t - Start capture", start_capture);
	command_insert('f', "\t - Stop capture", stop_capture);
	command_insert('l', "\t - List wave captures", list_captures);
	command_insert('s', "\t - Save wave capture", save_capture);  
	
	while(1) {
		putchar('>');
		fgets(line, sizeof line, stdin);
		char *command = strtok(line, " \n");
		char *name = strtok(NULL, " \n");
		if(command != NULL)
			command_execute(*command, name);
	}
}
