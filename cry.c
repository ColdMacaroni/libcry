#include "cry.h"
#include "reflection.h"
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

#define __USE_GNU
#include <dlfcn.h>
#include <link.h>

static void run_tests(struct test_list *list) {
	// TODO: Create a message queue and read it after child has exited.
	// NOTE: I think it's better to print output as tests run, easier to see
	// if something is stuck in a loop and stuff.
}

int main() {
	struct test_list tests = {NULL};

	_cry_find_tests(&tests);

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

	_cry_free_list(&tests);
}
