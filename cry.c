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
#include <unistd.h>

static void run_tests(struct test_list *list) {
	printf("TAP version 14\n");
	printf("1..%d\n", list->count);

	if (list->setup != NULL) {
		(*list->setup)(0);
	}

	// TODO: Create a message queue, then read it after child has exited.
	// NOTE: I think it's better to print output as tests run, easier to see
	// if something is stuck in a loop and stuff. Make sure to flush
	int idx = 1;
	for (struct test_node *n = list->head; n != NULL; n = n->next, idx++) {
		if (n->impl != NULL)
			(*n->impl)(0);

		// Currently any failed tests will abort the program, so if it reaches here, it passes.
		printf("ok %d", idx);

		if (n->desc != NULL)
			printf(" - %s", (*n->desc)());

		if (n->impl == NULL)
			printf(" # SKIP: no implementation");

		putchar('\n');
	}

	if (list->cleanup != NULL) {
		(*list->cleanup)(0);
	}
}

static void print_help() {
	printf("TODO: Help..\n");
}

static void list_tests(struct test_list *list) {
	if (list->count == 0) {
		printf("No tests found\n");
		return;
	}

	// This could be a log10 but i'd like to not link math if i can avoid it.
	int digits = 0;
	{
		int count = list->count;
		do {
			digits++;
		} while (count /= 10);
	}

	int idx = 1;
	for (struct test_node *n = list->head; n != NULL; n = n->next, idx++) {
		const char *desc = "(no description)";
		if (n->desc != NULL) {
			desc = (*n->desc)();
		}

		printf("%*d. %s - %s\n", digits, idx, n->name, desc);
	}
}

int main(int argc, char *argv[]) {
	struct test_list tests = {NULL, 0};
	_cry_find_tests(&tests);

	int opt;
	while ((opt = getopt(argc, argv, "hl")) != -1) {
		switch (opt) {
		case 'h':
		case '?':
			printf("Help");
			exit(1);
			break;
		case 'l':
			list_tests(&tests);
			exit(0);
			break;
		}
	}

	run_tests(&tests);

	_cry_free_list(&tests);
}
