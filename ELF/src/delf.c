// ELF reader

#include "delf_h1.h"
#include "delf_h2.h"

// Master fn for completely decoding and processing an ELF file.
bool readelf(const char *const filename) {
	fprintf(stdout, "FILE: %s\n", filename);
	
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		fprintf(stdout, "ERROR: Could not open \"%s\".\n", filename);
		return false;
	}
	
	ElfGeneric_Ehdr ge;
	if (!read_elf_header(&ge, fd)) {
		print_err_not_elf_file(filename);
		return false;
	}
	
	bool ret =
		print_elf_header(&ge) &&
		print_sections(&ge, fd);
	
	if (0 != close(fd))
		return print_error_msg("ERROR: Could not close file descriptor.\n");
	
	return ret;
}

void print_dividing_line(void) {
	fprintf(stdout, "\n");
	for (int i = 0; i < 80; i++)
		fprintf(stdout, "=");
	fprintf(stdout, "\n\n");
}

int main(int argc, char *argv[]) {
	argv[0][0] += argc - argc;
	
	if (argc < 2) {
		fprintf(stdout, "Usage: %s file1 file2 ...\n", argv[0]);
		return 0;
	}
	for (int i = 1; i < argc; i++) {
		readelf(argv[i]);
		print_dividing_line();
	}
}
