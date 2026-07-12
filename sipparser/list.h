#ifndef _LIST_H_
#define _LIST_H_



#ifdef __cplusplus
extern "C"
{
#endif

typedef struct node_t{
               void *next;
               void *element;
}node_t;


typedef struct list_t{
               int nb_elt;
               node_t *node;
}list_t;
        
        
/**
 * Initialise a list_t element.
 * NOTE: this element MUST be previously allocated.
 * @param li The element to initialise.
 */
 int list_init(list_t *li);
 
 /**
 * Free a list of element.
 * Each element will be free with the method given as the second parameter.
 * @param li The element to work on.
 * @param free_func The method that is able to release one element of the list.
 */
void list_special_free(list_t *li,void *(*free_func)(void *));

/**
 * Free a list of element where elements are pointer to 'char'.
 * @param li The element to work on.
 */

void listofchar_free(list_t *li);

/**
 * Get the size of a list of element.
 * @param li The element to work on.
 */

int  list_size(list_t *li);

/**
 * Check if the end of list is detected .
 * @param li The element to work on.
 * @param pos The index of the possible element.
 */

int list_eol(list_t *li, int pos);

/**
 * Add an element in a list.
 * @param li The element to work on.
 * @param element The pointer on the element to add.
 * @param pos the index of the element to add. (or -1 to append the element at the end)
 */

int list_add (list_t * li, void *element, int pos);

/**
 * Get an element from a list.
 * @param li The element to work on.
 * @param pos the index of the element to get.
 */

void *list_get(list_t *li,int pos);

/**
 * Remove an element from a list.
 * @param li The element to work on.
 * @param pos the index of the element to remove.
 */

int list_remove(list_t *li,int pos);


#ifdef __cplusplus
}
#endif

#endif
