#ifndef LIST_H
#define LIST_H
typedef struct node {
	struct node *next, *prev;
	void *data;
} Node;
// list creation returns pointer to first node of list
Node *list_create(); 
// insert rear receives list and new node to add to list
void list_insert_rear(Node *node, void *data); 
// checks if list is empty
int list_empty(Node *node); 
// deletes list
void list_delete(Node *list);
// iterate list to retrieve a specific wave capture
void * list_get_wave_capture(Node *list, int (*doit) (void *, int, int), int match); 
// iterate list of captures and display information
void list_foreach_captures(Node *list, void (*doit)(void *, int));
// iterate list to do some action
void list_foreach(Node *list, void (*doit)(void *));  
// iterate list to do some action and write to a file, returns value 0 in case of success
int list_foreach_write(Node *list, int (*doit)(void *, FILE *), FILE *file); 
#endif
