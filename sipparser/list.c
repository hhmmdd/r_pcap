
#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include "mem/mem.h"


int list_init ( list_t * li)
{
	li->nb_elt = 0;
	return 0;			
}


void list_special_free( list_t *li,void *(*free_func)(void *))
{
	int pos = 0;
	void * element;	

	if (li == NULL)
		return;
	while(!list_eol(li,pos))
	{
		element = (void *)list_get(li,pos);
		list_remove(li,pos);
		if (free_func != 0) free_func(element);
		pkg_free(element);
	}
}

void listofchar_free(list_t *li)
{
	int pos = 0;
	char *chain;

	if (li == 0)
		return;
	while(!list_eol(li,pos))
	{
		chain = (char *)list_get(li,pos);
		list_remove(li,pos);
		pkg_free(chain);
	}
}

int	list_size(list_t *li)
{
	if(li != 0)
		return li->nb_elt;
	else
		return -1;
}

int list_eol(list_t *li,int i)
{
	if (i < li->nb_elt)
		return 0;
	return 1;
}

int list_add(list_t *li,void *element,int pos)
{
	node_t *ntmp;
	node_t *nextnode;

	int i = 0;
    
	/* insert at the end  */
	if (pos == -1 || pos >= li->nb_elt){
		pos = li->nb_elt;
	}

	if (li->nb_elt == 0){
		li->node = (node_t *)pkg_malloc(sizeof(node_t));
		li->node->element = element;
		li->nb_elt++;
		return li->nb_elt;
	}
	ntmp = li->node;
    
	/* pos = 0 insert before first elt  */
	if (pos == 0){
		li->node = (node_t *)pkg_malloc(sizeof(node_t));
		li->node->element = element;
		li->node->next = ntmp;
		li->nb_elt++;
		return li->nb_elt;
	}

	while(pos > i+1){
		i++;
		ntmp = (node_t *)ntmp->next;
	}

	/* if pos==nb_elt next node does not exist  */
	if (pos == li->nb_elt)
	{
		ntmp->next = (node_t *) pkg_malloc(sizeof (node_t));
		ntmp = (node_t *) ntmp->next;
		ntmp->element = element;
		li->nb_elt++;
		return li->nb_elt;
	}
	/* here pos==i so next node is where we want to insert new node */

	nextnode = (node_t *) ntmp->next;
	ntmp->next = (node_t *) pkg_malloc(sizeof (node_t));
	ntmp = (node_t *) ntmp->next;
	ntmp->element = element;
	ntmp->next = nextnode;
	li->nb_elt++;
	
	return li->nb_elt;
}

void *list_get(list_t *li,int pos)
{
	node_t *ntmp;
	int i = 0;

	if (pos < 0 || pos >= li->nb_elt)
		return 0;

	ntmp = li->node;

	while (pos > i){
		i++;
		ntmp = (node_t *)ntmp->next;
	}
	return ntmp->element;
}

int list_remove(list_t *li,int pos)
{
	node_t *ntmp;
	node_t *remnode;
	int i = 0;

	if (pos < 0 || pos >= li->nb_elt)
		return -1;

	ntmp = li->node;

	if (pos == 0){
		li->node = (node_t *)ntmp->next;
		li->nb_elt--;
		pkg_free(ntmp);
		return li->nb_elt;
	}

	while(pos > i + 1){
		i++;
		ntmp = (node_t *) ntmp->next;
	}
	remnode = (node_t *) ntmp->next;
	ntmp->next = ((node_t *) ntmp->next)->next;
	pkg_free(remnode);
	li->nb_elt--;
	return li->nb_elt;
}





