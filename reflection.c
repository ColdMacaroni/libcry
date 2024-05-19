#include "reflection.h"
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

#define __USE_GNU
#include <dlfcn.h>
#include <link.h>

/**
 * Mallocs and sets up a new node
 */
static struct test_node *new_node(char *name, impl_func *impl,
                                  desc_func *desc) {
	struct test_node *t = malloc(sizeof(*t));

	t->name = name;
	t->impl = impl;
	t->desc = desc;
	t->next = NULL;

	return t;
}

static int find_symbols_64(struct dl_phdr_info *info, size_t size,
                           struct test_list *list) {

	return 0;
}

/** Uses the base addr to grab the ehdr. */
static Elf64_Ehdr *get_ehdr(struct dl_phdr_info *info) {
	return (Elf64_Ehdr *)info->dlpi_addr;
}

static int phdr_callback(struct dl_phdr_info *info, size_t size, void *data_) {
	// We only want to run this callback once.
	// The first time, it's our phdr.
	// We can just set our data to null to signal this
	struct test_list **data = data_;
	if (*data == NULL)
		return 1;

	struct test_list *list = *data;
	*data = NULL;

	// different architectures have different alignments.....
	// It's okay to read the 64 bit one for both this bc these
	// bytes match up.
	switch (get_ehdr(info)->e_ident[EI_CLASS]) {
	case ELFCLASS64:
		return find_symbols_64(info, size, list);
	case ELFCLASS32:
		fprintf(stderr, "Sorry!! 32bit isn't supported currently. "
		                "If you really do need it, please open an issue!\n");
		exit(1);
		break;
	default:
		fprintf(stderr, "What a strange architecture... or I misread a byte. "
		                "Either way, bye !\n");
		fprintf(stderr, "If your ELF is not malformed, please open an issue!\n");
		exit(4);
	}

	return 0;
}

void _cry_find_tests(struct test_list *list) {
	// https://stackoverflow.com/questions/2694290/returning-a-shared-library-symbol-table/2694373#2694373
	void *handle = dlopen(NULL, RTLD_LAZY);

	dl_iterate_phdr(phdr_callback, &list);

	dlclose(handle);
}

void _cry_free_list(struct test_list *list) {
	fprintf(stderr, "TODO! free list\n");
}
