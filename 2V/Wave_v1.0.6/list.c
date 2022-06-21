#include <stdlib.h>
#include <stdio.h>

#include "list.h"

Node *list_create() {
	Node *node = malloc(sizeof *node);
	if (NULL == node) {
		fprintf(stderr, "Out of memory\n");
		exit(-1);
	}
	node->next = node->prev = node;
	node->data = NULL;
	return node;
}

void list_insert_rear(Node *list, void *data) {
	Node *new = malloc(sizeof *new);
	if (NULL == new) {
		fprintf(stderr, "Out of memory\n");
		exit(-1);
	}
	new->next = list;
	new->prev = list->prev;
	list->prev->next = new;
	list->prev = new;

	new->data = data;
}

int list_empty(Node *node) {
	return node->next == node;
}

void list_delete(Node *list) {
	Node *next;
	for(Node *p = list->next; list != p; p = next) {
		next = p->next;
		free(p);
	}
}

void * list_get_wave_capture(Node *list, int (*doit) (void *, int, int), int match) {
	int i = 1;
	for(Node *p = list->next; list != p; p = p->next, i++) {
		if(doit(p->data, i, match)) {
			return p->data; // if found returns Wave node
		}
	}
	return NULL;
}

void list_foreach_captures(Node *list, void (*doit)(void *, int)) {
	int i = 1; 
	for(Node *p = list->next ; list != p; p = p->next, i++)
		doit(p->data, i);
}

void list_foreach(Node *list, void (*doit)(void *)) {
	for(Node *p = list->next; list != p; p = p->next)
		doit(p->data);
}

/*int list_foreach_write(Node *list, int (*doit)(void *, FILE *), FILE *file) { // old list_foreach_write()
	int res;
	for(Node *p = list->next; list != p; p = p->next) {
		res = doit(p->data, file);
		if(res < 0)
			return res; // in case something goes wrong
	}
	return 0; // if all went right returns code 0 (success)
}*/

int list_foreach_write(Node *list, int (*doit)(void *, FILE *), FILE *file) {
	int res;
	for(Node *p = list->next; list != p; p = p->next) {
		res = doit(p->data, file);
		if(res != 1)
			return res; // in case something goes wrong
	}
	return 1; // if all went right returns code 1 (success)
}
