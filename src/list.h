#ifndef LIST_H
#define LIST_H

typedef struct cell *list;

struct cell
{
  void *element;
  list next;
};

extern list cons(void *element, list l);
extern list cdr_and_free(list l);
extern int rm_elem(list* l, void* el, int (*f)(void*, void*), int flag);
extern void* search_elem(list l, void* el, int (*cmp)(void*, void*));
extern void free_list(list* l, void (*free_func)(void*));
extern void add_elem(list* l, void* elem);
extern void print_list(list l, void (*print_func)(void*));

#endif
