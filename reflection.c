#include "reflection.h"
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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

/** Uses the base addr to grab the elf header. */
// static Elf64_Ehdr *get_ehdr(struct dl_phdr_info *info) {
// 	return (Elf64_Ehdr *)info->dlpi_addr;
// }
//
/** Uses the ehdr data to grab the section header.
 * ehdr must point to the start of the elf.
 */
static Elf64_Shdr *get_shdrs(Elf64_Ehdr *ehdr) {
	return (Elf64_Shdr *)((void *)ehdr + ehdr->e_shoff);
}

static char *get_strtab(Elf64_Ehdr *ehdr) {
	Elf64_Shdr *sh_strtab = &get_shdrs(ehdr)[ehdr->e_shstrndx];
	return (char *)((void *)ehdr + sh_strtab->sh_offset);
}

//
static int find_symbols_64(Elf64_Ehdr *ehdr, struct test_list *list) {
	if (sizeof(Elf64_Shdr) != ehdr->e_shentsize) {
		fprintf(stderr, "Elf64_Shdr is different than e_shentsize. Unsure how "
		                "to proceed!\n");
		exit(6);
	}

	if (ehdr->e_shstrndx == SHN_UNDEF) {
		fprintf(stderr,
		        "strtab index is undefined. Unsure how to proceed. "
		        "If your ELF file is not broken, please open an issue\n");
		exit(79);
	}

	Elf64_Shdr *shdrs = get_shdrs(ehdr);
	char *strtab = get_strtab(ehdr);

	// List types for debugging purposes
	for (int i = 0; i < ehdr->e_shnum; i++) {
		Elf64_Shdr *shdr = &shdrs[i];

		// We only care about this
		if (shdr->sh_type != SHT_SYMTAB)
			continue;

		printf("Found symtab at %d\n", i);
		printf("%s\n", &strtab[shdr->sh_name]);
	}

	return 0;
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

	switch (ehdr->e_ident[EI_CLASS]) {
	case ELFCLASS64:
		find_symbols_64(ehdr, list);
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
	if (munmap(ehdr, buf.st_size) < 0) {
		perror("munmap");
	}

	if (close(fd) < 0) {
		perror("close");
	}
}

void _cry_find_tests(struct test_list *list) {
	find_symbols(list);

	// https://stackoverflow.com/questions/2694290/returning-a-shared-library-symbol-table/2694373#2694373
	// TODO:
	// void *handle = dlopen(NULL, RTLD_LAZY);
	//
	// dlclose(handle);
}

void _cry_free_list(struct test_list *list) {
	fprintf(stderr, "TODO! free list\n");
}
