#include <alsa/asoundlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/dlist.h"
#include "utils/utils.h"
#include "wavelib/wave.h"

static Node *commands, *wave_files, *queue;

typedef struct command {
    char letter;
    char *args;
    char *description;
    void (*action)(char *);
} Command;

Command *new_command(char letter, char *args, char *description, void (*action)(char *)) {
    Command *cmd = malloc(sizeof *cmd);
    if (cmd == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(-1);
    }
    cmd->letter = letter;
    cmd->args = args;
    cmd->description = description;
    cmd->action = action;
    return cmd;
}

void sorted_insert_if_matches(const char *it, void *context) {
    if (string_match(context, it)) {
        char *text = malloc(strlen(it) + 1);
        strcpy(text, it);
        if (wave_files->next == wave_files) {
            // List is empty
            list_insert_front(wave_files, (void *)text);
            return;
        }
        for (Node *p = wave_files->next; p != wave_files; p = p->next) {
            if (strcmp(it, p->data) <= 0) {
                list_insert_front(p->prev, (void *)text);
                return;
            }
        }
        // Insert at end
        list_insert_rear(wave_files, (void *)text);
    }
}

void free_nodes_and_data(Node *node) {
    Node *next;
    for (Node *p = node->next; p != node; p = next) {
        next = p->next;
        free(p->data);
        free(p);
    }
    free(node);
}

void show_wave_files() {
    printf("\nWave files found:\n");
    int idx = 1;
    for (Node *p = wave_files->next; p != wave_files; p = p->next, idx++) {
        printf(" %d. '%s'\n", idx, (char *)p->data);
    }
    printf("\n");
}

void add_to_queue(char *wave_index) {
    if (wave_index == NULL) {
        printf("\nInvalid\n\n");
        return;
    }

    int idx;
    sscanf(wave_index, "%d", &idx);
    if (idx <= 0) {
        printf("\nInvalid\n\n");
        return;
    }

    // Find wave to add to queue
    Node *node = wave_files;
    for (int i = 0; i < idx; i++) {
        node = node->next;
        if (node == wave_files) {
            printf("\nInvalid\n\n");
            return;
        }
    }

    // Add wave to queue
    char *file_path = malloc(strlen(node->data) + 1);
    if (file_path == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(-1);
    }
    strcpy(file_path, node->data);
    list_insert_rear(queue, file_path);
}

void clear_queue(char *unused) {
    free_nodes_and_data(queue);
    queue = list_create();
}

void show_queue(char *unused) {
    if (queue == queue->next) {
        printf("\nQueue is empty\n");
    } else {
        printf("\nQueue:\n");
        int idx = 1;
        for (Node *p = queue->next; p != queue; p = p->next, idx++) {
            printf(" %d. '%s'\n", idx, (char *)p->data);
        }
    }
    printf("\n");
}

void play_queue(char *unused) {
    printf("\n");
    for (Node *p = queue->next; p != queue; p = p->next) {
        const char *file_path = (char *)p->data;
        Wave *wave = wave_load(file_path);
        printf("Playing '%s'...\n", file_path);
        wave_play(wave);
        wave_destroy(wave);
    }
}

void help(char *unused) {
    printf("\nHelp:\n");
    for (Node *p = commands->next; p != commands; p = p->next) {
        Command *cmd = p->data;
        printf(" %c", cmd->letter);
        if (cmd->args != NULL) {
            printf(" %s", cmd->args);
        }
        printf(" %s\n", cmd->description);
    }
    printf("\n");
}

void leave_program(char *unused) {
    free_nodes_and_data(commands);
    free_nodes_and_data(wave_files);
    free_nodes_and_data(queue);
    exit(0);
}

void command_execute(char c, char *param) {
    for (Node *p = commands->next; p != commands; p = p->next) {
        Command *cmd = p->data;
        if (cmd->letter == c) {
            cmd->action(param);
            return;
        }
    }
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <directory>\n", argv[0]);
        return -1;
    }

    commands = list_create();
    wave_files = list_create();
    queue = list_create();

    file_tree_foreach(argv[1], &sorted_insert_if_matches, "*.wav");
    show_wave_files();

    Command *cmd;
    cmd = new_command('a', "<wave_index>", "Add to queue", &add_to_queue);
    list_insert_rear(commands, cmd);
    cmd = new_command('q', NULL, "Show queue", &show_queue);
    list_insert_rear(commands, cmd);
    cmd = new_command('p', NULL, "Play queue", &play_queue);
    list_insert_rear(commands, cmd);
    cmd = new_command('c', NULL, "Clear queue", &clear_queue);
    list_insert_rear(commands, cmd);
    cmd = new_command('h', NULL, "Help", &help);
    list_insert_rear(commands, cmd);
    cmd = new_command('e', NULL, "Exit Application", &leave_program);
    list_insert_rear(commands, cmd);

    char line[100];
    while (1) {
        printf("> ");
        fgets(line, sizeof line, stdin);
        char *command = strtok(line, " \n");
        char *arg = strtok(NULL, " \n");
        if (command != NULL)
            command_execute(*command, arg);
    }

    return 0;
}