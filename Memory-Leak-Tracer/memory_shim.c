#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>

void __attribute__ ((constructor)) init(void);
void __attribute__ ((destructor)) cleanup(void);
void *(*original_malloc)(size_t size) = NULL;
void (*original_free)(void *ptr) = NULL;

/* Struct node used to implement linked list that will store
   the size of mem malloced along with its associated ptr.  */
struct node {
	int mem_size;
	void *mem_ptr;
	struct node *next;
};

int list_length = 0;
int leak_count = 0;
struct node *root;
struct node *current;

/* Prints out information regarding any leaked memory. */
void cleanup(void) {
	int sum = 0, i = 0, leak_count = 0;
	int leaks[list_length];
	struct node *temp = root;

	/* 1. Loop through list while updating the total number
	 *    of leaks and the sum of the memory leaks. */
	for(i = 0; i < list_length; i++) {
	   if(temp->mem_ptr != NULL) {
	      sum += temp->mem_size;
	      leaks[leak_count] = temp->mem_size;
	      leak_count++;
	   }
	   temp = temp->next;
	}

	/* 2. Print out the leaks. */
	for(i = 0; i < leak_count; i++) {
	   fprintf(stderr, "LEAK\t %i\n", leaks[i]);
	}

	/* 3. Print out the total number of leaks. */
	fprintf(stderr, "TOTAL\t %i\t  %i\n", leak_count, sum);
}

/* sets the wrapper's pointers to point to malloc and free */
void init(void) {
	if(original_malloc == NULL) {
	   original_malloc = dlsym(RTLD_NEXT, "malloc");
	}
	if(original_free == NULL) {
	   original_free = dlsym(RTLD_NEXT, "free");
	}
}

/* Retrieves the requested memory, stores the information on said memory
 * block as a node in the linked_list, and returns a pointer to the memory.*/
void *malloc(size_t size) {

	/* 1. Call original malloc to retrieve the requested memory. */
	void *ptr = original_malloc(size);

	/* 2a. If the list is empty, create a new node and set it as the root.
	 *     Store the memory size and the pointer to the memory. */
	if(list_length == 0) {
	   root = (struct node *) original_malloc(sizeof(struct node));
	   root->mem_size = (int) (size);
	   root->mem_ptr = ptr;
	   root->next = NULL;
	   current = root;
	} 
	
	/* 2b. Otherwise, create a new node and append it to the list.
	 *     Store the memory size and the pointer to the memory. */
	else {
	   struct node *new_node = (struct node *) original_malloc(sizeof(struct node));
	   new_node->mem_size = (int) (size);
	   new_node->mem_ptr = ptr;
	   new_node->next = NULL;
	   current->next = new_node;
	   current = new_node;
	}

	/* 3. Increment the list_length and return the memory pointer. */
	list_length++;
	return ptr;
}

/* Iterates through the list to find the node containing the ptr
   to be freed. Sets the nodes ptr to "NULL" to indicate that
   the memory has now been freed. Finally, calls original free. */
void free(void *ptr) {
	int i;
	struct node *temp = root;
	for(i = 0; i < list_length; i++) {
	   if(temp->mem_ptr == ptr) {
	      temp->mem_size = 0;
	      temp->mem_ptr = NULL;
	   }
	   temp = temp->next;
	}
	original_free(ptr);
}
