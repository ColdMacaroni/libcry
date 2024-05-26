/* libcry - A C testing framework that doesn' make you want to cry
 * Copyright (C) 2024  babalark <babalark@proton.me>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#pragma once
#ifndef __REFLECTION_H__
#define __REFLECTION_H__

#include <mqueue.h>
typedef void(impl_func)(mqd_t);
typedef const char *(desc_func)(void);

/* linked list node */
struct test_node {
	/* This field should NEVER be null */
	char *name;
	impl_func *impl;
	desc_func *desc;
	struct test_node *next;
};

/* singly linked list for storing all the function names */
struct test_list {
	struct test_node *head;
	int count;
};

/* uses ELF trickery to find all the tests */
void _cry_find_tests(struct test_list *list);

/* Free all the nodes of the list. */
void _cry_free_list(struct test_list *list);

#endif /* __REFLECTION_H__ */
