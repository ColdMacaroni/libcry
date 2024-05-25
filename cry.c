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

#include "cry.h"
#include "reflection.h"
#include <stdio.h>
#include <stdlib.h>

static void run_tests(struct test_list *list) {
	printf("TAP version 14\n");
	printf("1..%d\n", list->count);

	// TODO: Create a message queue, then read it after child has exited.
	// NOTE: I think it's better to print output as tests run, easier to see
	// if something is stuck in a loop and stuff. Make sure to flush
	int idx = 1;
	for (struct test_node *n = list->head; n != NULL; n = n->next, idx++) {
		printf("Running %s - %s... ", n->name,
		       n->desc == NULL ? "(no description)" : (*n->desc)());
		fflush(stdout);
		if (n->impl == NULL) {
			printf("skipped\n");
		} else {
			(*n->impl)(0);
			printf("passed\n");
		}
	}
}

int main() {
	struct test_list tests = {NULL, 0};

	_cry_find_tests(&tests);

	// TODO: Actual nice command line interface

	int idx = 1;
	if (tests.head == NULL)
		printf("No tests found\n");
	else
		for (struct test_node *n = tests.head; n != NULL; n = n->next, idx++) {
			printf("%d. %s\t", idx, n->name);
			if (n->desc != NULL) {
				char *desc = (*n->desc)();
				printf("%s", desc);
			} else {
				printf("(no description)");
			}
			putchar('\n');
		}

	run_tests(&tests);

	_cry_free_list(&tests);
}
