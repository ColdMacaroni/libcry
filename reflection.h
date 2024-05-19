#pragma once
#ifndef __REFLECTION_H__
#define __REFLECTION_H__

#include <mqueue.h>
typedef void(impl_func)(mqd_t);
typedef char *(desc_func)(void);


/* linked list node */
struct test_node {
	/* This filed should NEVER be null */
	char *name;
	impl_func *impl;
	desc_func *desc;
	struct test_node *next;
};

/* doubly linked list for storing all the function names */
struct test_list {
	struct test_node *head;
	struct test_node *tail;
};

/* uses ELF trickery to find all the tests */
void _cry_find_tests(struct test_list *list);

/* Free all the nodes of the list. */
void _cry_free_list(struct test_list *list);


#endif /* __REFLECTION_H__ */
