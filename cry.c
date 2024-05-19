#include "cry.h"
#include "reflection.h"
#include <stdio.h>
#include <stdlib.h>

static void run_tests(struct test_list *list) {
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
	struct test_list tests = {NULL};

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
