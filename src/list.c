#include "list.h"
#include <stdlib.h>
#include <stdio.h>

list cons(void *element, list l)
{
	list temp = malloc(sizeof(struct cell));
	temp->element = element;
	temp->next = l;
	return temp;
}

list cdr_and_free(list l)
{
	list temp = l->next; 
	free(l);
	return temp;
}

// return -1 on trying to remove from empty list
// returns 0 on succes
int rm_elem(list* l, void* el, int (*cmp)(void*, void*), int flag) {

	if(*l == NULL) {
		return -1;
	}

	list p;
	list aux;
	aux = NULL;

	for(p = *l; p != NULL; aux = p, p = p->next) {
		if((*cmp)(p->element, el) == 0) {
			break;
		}
	}

	if(p == NULL) {
		// nu exista elementul de sters
		return -1;
	}
	
	if(flag) {
		free(p->element);
	}
	// daca pe ecapul listei
	if(p == *l) {
		*l = (*l)->next;
		free(p);
		return 0;
	}

	// daca nu e capul listei inseamna ca aux nu e null
	aux->next = p->next;
	free(p);


	return 0;
}

// searches for el in the list and returns the first apparition if found
// return NULL otherwise
void* search_elem(list l, void* el, int (*cmp)(void*, void*)) {
	list p = l;
	while(p != NULL) {
		if((*cmp)(p->element, el) == 0) {
			return p->element;
		}
		//printf("%s", (char*)((p->element) + 5));
		p = p->next;
	}

	return NULL;
}

void free_list(list* l, void (*free_func)(void**)) {
	list aux;
	while(*l != NULL) {
		aux = *l;
		(*free_func)(&(aux->element));
		*l = (*l)->next;
		free(aux);
	}
}

void add_elem(list* l, void* elem) {
	if(*l == NULL) {
		*l = (list) calloc(1, sizeof(struct cell));
		(*l)->element = elem;
		return;
	}

	list aux = (list) calloc(1, sizeof(struct cell));
	aux->next = (*l);
	aux->element = elem;
	*l = aux;
}

void print_list(list l, void (*print_func)(void*)) {
	int counter = 0;
	while(l != NULL) {
		// printf("counter[%d]\n", counter);
		print_func(l->element);
		l = l->next;
		counter++;
	}
	//printf("AM PRINTAT [%d] ELEMENTE\n", counter);
}
