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

#include "reflection.h"
#include <assert.h>
#include <dlfcn.h>
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Mallocs and sets up a new node
 */
static struct test_node *new_node(char *name) {
	struct test_node *t = malloc(sizeof(*t));

	if (t == NULL)
		return NULL;

	t->name = name;
	t->impl = NULL;
	t->desc = NULL;
	t->next = NULL;

	return t;
}

enum symbol_type {
	UNKNOWN,
	IMPL,
	DESC,
};

/**
 * Converts the given string to a symbol_type
 * string | enum
 * -------+-----
 *  impl  | IMPL
 *  desc  | DESC
 *
 *  any other returns an UNKNOWN.
 */
static enum symbol_type str_to_type(const char *type) {
	if (strcmp(type, "impl") == 0) {
		return IMPL;
	} else if (strcmp(type, "desc") == 0) {
		return DESC;
	} else {
		return UNKNOWN;
	}
}

/**
 * Tries to find the node with the given name, or creates one if not found.
 * Only returns null if it was unable to malloc.
 * Keeps the list in order
 */
static struct test_node *find_node(struct test_list *list, char *name) {
	// special case
	if (list->head == NULL) {
		struct test_node *n = new_node(name);

		if (n == NULL)
			return NULL;

		list->count++;

		list->head = n;
		return n;
	}

	// Try to find mathcing name
	for (struct test_node *cur = list->head; cur != NULL; cur = cur->next) {
		if (strcmp(cur->name, name) == 0)
			return cur;
	}

	struct test_node *n = new_node(name);

	if (n == NULL)
		return NULL;

	list->count++;

	struct test_node *other = list->head;

	// Special case where it's the new first element
	int cmp = strcmp(n->name, other->name);
	if (cmp < 0) {
		n->next = other;
		list->head = n;
		return n;
	}

	// append sorted
	while (1) {
		if (other->next == NULL) {
			other->next = n;
			return n;
		}

		cmp = strcmp(n->name, other->next->name);
		if (cmp < 0) {
			n->next = other->next;
			other->next = n;
			return n;
		}

		other = other->next;
	}
}

/** Uses the ehdr data to grab the section header.
 * ehdr must point to the start of the elf.
 */
static Elf64_Shdr *get_shdrs(Elf64_Ehdr *ehdr) {
	return (Elf64_Shdr *)((char *)ehdr + ehdr->e_shoff);
}

static char *get_strtab(Elf64_Ehdr *ehdr, Elf64_Shdr *shdr) {
	Elf64_Shdr *sh_strtab = &get_shdrs(ehdr)[shdr->sh_link];
	return (char *)ehdr + sh_strtab->sh_offset;
}

static Elf64_Sym *get_symtab(Elf64_Ehdr *ehdr, Elf64_Shdr *sh_symtab) {
	return (Elf64_Sym *)((char *)ehdr + sh_symtab->sh_offset);
}

/* Finds all the symbols with the format _cry_test_$<name>$_<type>,
 * and pushes them to the end of the list.
 */
static void find_symbols_64(Elf64_Ehdr *ehdr, struct test_list *list,
                            void *dlhandle) {
	if (sizeof(Elf64_Shdr) != ehdr->e_shentsize) {
		fprintf(stderr, "Elf64_Shdr is different than e_shentsize. Unsure how "
		                "to proceed!\n");
		exit(6);
	}

	Elf64_Shdr *shdrs = get_shdrs(ehdr);
	Elf64_Shdr *sh_symtab = NULL;

	// Find the symtab to extract all those symbol names
	for (int i = 0; i < ehdr->e_shnum; i++) {
		Elf64_Shdr *shdr = &shdrs[i];

		// We only care about this
		if (shdr->sh_type != SHT_SYMTAB)
			continue;

		sh_symtab = shdr;
		break;
	}

	if (sh_symtab == NULL) {
		fprintf(stderr, "symtab not found. Is your ELF okay?\n");
		exit(10);
	}

	char *strtab = get_strtab(ehdr, sh_symtab);

	Elf64_Sym *symtab = get_symtab(ehdr, sh_symtab);
	Elf64_Xword symtab_len = sh_symtab->sh_size / sh_symtab->sh_entsize;

	for (Elf64_Xword sym_idx = 0; sym_idx < symtab_len; sym_idx++) {
		Elf64_Sym *sym = &symtab[sym_idx];

		char *sym_name = &strtab[sym->st_name];
		char *name;
		char *type_s;

		// FIXME: This is very ugly, tidy up later. Split into funcs or smth
		if (strcmp(sym_name, "_cry$setup") == 0) {
			void *symbol = dlsym(dlhandle, sym_name);
			if (symbol == NULL) {
				fprintf(stderr, "Error reading symbol %s\n", sym_name);
				// TODO: If errno is not set, then ask if they compiled with
				// rdynamic
				perror("dlsym");
				exit(99);
			}

			list->setup = symbol;
			continue;
		} else if (strcmp(sym_name, "_cry$cleanup") == 0) {
			void *symbol = dlsym(dlhandle, sym_name);
			if (symbol == NULL) {
				fprintf(stderr, "Error reading symbol %s\n", sym_name);
				// TODO: If errno is not set, then ask if they compiled with
				// rdynamic
				perror("dlsym");
				exit(99);
			}
			list->cleanup = symbol;
			continue;
		}

		// Must stick to this format.
		if (sscanf(sym_name, "_cry_test_$%m[^$]$_%ms", &name, &type_s) != 2)
			continue;

		enum symbol_type type = str_to_type(type_s);

		// Not one of our symbols.
		if (type == UNKNOWN)
			continue;

		void *symbol = dlsym(dlhandle, sym_name);
		if (symbol == NULL) {
			fprintf(stderr, "Error reading symbol %s\n", sym_name);
			// TODO: If errno is not set, then ask if they compiled with
			// rdynamic
			perror("dlsym");
			exit(99);
		}

		struct test_node *n = find_node(list, name);

		if (n == NULL) {
			fprintf(
			    stderr,
			    "Couldn't allocate a linked list node for %s, stopping early\n",
			    sym_name);
			free(type_s);
			free(name);
			return;
		}

		switch (type) {
		case IMPL:
			n->impl = symbol;
			break;
		case DESC:
			n->desc = symbol;
			break;

		case UNKNOWN:
			// should be unreachable
			break;
		}

		// Don't free name because it's in the linked list
		free(type_s);
	}

	return;
}

/* Populates the given list with symbols. Both its pointers should be null.
 * Will exit on any errors.
 * XXX: Should this exit() or return an error that can be reported in TAP?
 */
static void find_symbols(struct test_list *list) {
	// This is a little cursed, but we're gonna parse ourselves.
	// If you can think of a better more libc way of doing this that isn't
	// just straight up raw reading the ELF, let me know!
	int fd = open("/proc/self/exe", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Unable to inspect self via /proc/self/exe\n");
		perror("open");
		exit(1);
	}

	struct stat buf;
	fstat(fd, &buf);

	Elf64_Ehdr *ehdr = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (ehdr == NULL || ehdr == MAP_FAILED) {
		fprintf(stderr, "Failed to mmap /proc/self/exe\n");
		perror("mmap");
		close(fd);
		exit(1);
	}

	// Check magic bytes
	if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
	    ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
		fprintf(stderr,
		        "You've somehow managed to run this as not an ELF file. "
		        "Impressive, but no supported\n");

		munmap(ehdr, buf.st_size);
		close(fd);
		exit(42);
	}

	if (ehdr->e_version != 1) {
		fprintf(stderr, "Unsupported ELF version %u\n", ehdr->e_version);

		munmap(ehdr, buf.st_size);
		close(fd);
		exit(42);
	}

	void *handle = dlopen(NULL, RTLD_LAZY);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't dlopen self\n");
		perror("dlopen");
		munmap(ehdr, buf.st_size);
		close(fd);
		exit(51);
	}

	switch (ehdr->e_ident[EI_CLASS]) {
	case ELFCLASS64:
		find_symbols_64(ehdr, list, handle);
		break;
	case ELFCLASS32:
		fprintf(stderr, "Sorry!! 32bit isn't supported currently. "
		                "If you really do need it, please open an issue!\n");

		munmap(ehdr, buf.st_size);
		close(fd);
		exit(1);
		break;
	default:
		fprintf(stderr, "What a strange architecture... or I misread a byte. "
		                "Either way, bye !\n");
		fprintf(stderr,
		        "If your ELF is not malformed, please open an issue!\n");

		munmap(ehdr, buf.st_size);
		close(fd);
		exit(4);
	}

	// Technically we can still keep going, right?
	if (dlclose(handle) < 0) {
		perror("dlclose");
	}

	if (munmap(ehdr, buf.st_size) < 0) {
		perror("munmap");
	}

	if (close(fd) < 0) {
		perror("close");
	}
}

void _cry_find_tests(struct test_list *list) { find_symbols(list); }

void _cry_free_list(struct test_list *list) {
	if (list == NULL)
		return;

	for (struct test_node *cur = list->head; cur != NULL;) {
		struct test_node *tmp = cur;
		cur = cur->next;

		free(tmp->name);
		free(tmp);
	}

	list->head = NULL;
}
