// utility fns

#include "delf_h1.h"
#include "delf_h2.h"

// copied fn. "uint" here is actually "ulong" on IA-32.
uint elf_hash(const unsigned char *name) {
	uint h = 0, g = 0;
	while (*name) {
		h = (h << 4) + *name++;
		if ((g = (h & 0xf0000000)))
			h ^= g >> 24;
		h &= ~g;
	}
	return h;
}

// unused fn
int charToDigit(const char ch) {
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	return -1;
}

// unused fn
int strToInt(const char *s, int *const n) {
	if (0 == strlen(s) || 10 < strlen(s)) return -1;  // bad string
	int acc = 0;  // number accumulator
	int scale = 1;
	for (int i = strlen(s)-1; i >= 0; i--) {
		int digit = charToDigit(s[i]);
		if (digit == -1 ||
			digit > INT_MAX / scale ||
			digit * scale > INT_MAX - acc ||
			(0 != i && scale > INT_MAX / 10)) return -1;  // bad string
		acc += digit * scale;
		if (0 != i) scale *= 10;
	}
	*n = acc;
	return 0;
}

// Print an error that filename is not an ELF file. Always returns false.
bool print_err_not_elf_file(const char *const filename) {
	fprintf(stdout, "ERROR: \"%s\" is not an ELF file.\n", filename);
	return false;
}

// Return true if the host is little-endian.
bool host_is_le(void) {
	const int word = 0x000000ff;
	if (*(const uchar *const)(&word) == 0xff)
		return true;
	return false;
}

// Reverse the order of the bytes in an array. Inputs must be valid!
void reverse_bytes_direction(void *const bytes, const size_t howmany) {
	char *data = (char *)bytes;
	for (uint i = 0; i < howmany / 2; i++) {
		const char left = data[i];
		data[i] = data[howmany - 1 - i];
		data[howmany - 1 - i] = left;
	}
}

// Print a single byte.
void print_byte_as_hex(const uchar byte) {
	fprintf(stdout, "%.2x ", byte);
}

// Print nbytes bytes. Input must be valid!
void print_sz_bytes_le(const char *bytes, uint nbytes) {
	for (int i = nbytes - 1; i >= 0; i--)
		print_byte_as_hex(*(bytes + i));
}

// Read nbytes bytes from fd into data. Input must be valid!
bool read_into(void *data, ssize_t nbytes, int fd) {
	if (nbytes < 0)
		return false;
	ssize_t cur_off, max_off;
	if (!get_fd_max_off(fd, &max_off) || !get_fd_cur_off(fd, &cur_off))
		return false;
	if (cur_off > max_off - nbytes)
		return false;
	
	ssize_t has_read = 0;
	while (has_read < nbytes) {
		if ((ssize_t)((char *)data) > (ssize_t)(~0ull >> 1) - has_read) {
			memset(data, 0xFF, nbytes);
			return false;
		}
		int ret = read(fd, (char *)data + has_read, nbytes - has_read);
		if (ret <= 0 || has_read > (ssize_t)(~0ull >> 1) - ret) {
			memset(data, 0xFF, nbytes);
			return false;
		}
		has_read += ret;
	}
	return true;
}

// Return true if the ELF is 64-bit. Input must be valid!
bool is_Elf64(const ElfGeneric_Ehdr *const ge) {
	if (ge->ehdr32.common.ident.ei_class == ELFCLASS64)
		return true;
	return false;
}

// Convert the endianness of the ELF header multibyte fields to match the host
void maybe_reverse_e_endianness(ElfGeneric_Ehdr *const e) {
	uchar ei_data = e->ehdr32.common.ident.ei_data;
	if (host_is_le() != (ei_data == ELFDATA2MSB))
		return;

	int addr_size = sizeof(uint);
	void
		*pentry = &e->ehdr32.entry, *pphoff = &e->ehdr32.phoff,
		*pshoff = &e->ehdr32.shoff;
	uint *flags = &e->ehdr32.flags;
	ushort
		*ehsize = &e->ehdr32.ehsize,
		*phentsize = &e->ehdr32.phentsize, *phnum = &e->ehdr32.phnum,
		*shentsize = &e->ehdr32.shentsize, *shnum = &e->ehdr32.shnum,
		*shstrndx = &e->ehdr32.shstrndx;
	if (e->ehdr32.common.ident.ei_class == ELFCLASS64) {
		addr_size = sizeof(ull);
		pentry = &e->ehdr64.entry;
		pphoff = &e->ehdr64.phoff;
		pshoff = &e->ehdr64.shoff;
		flags = &e->ehdr64.flags;
		ehsize = &e->ehdr64.ehsize;
		phentsize = &e->ehdr64.phentsize;
		phnum = &e->ehdr64.phnum;
		shentsize = &e->ehdr64.shentsize;
		shnum = &e->ehdr64.shnum;
		shstrndx = &e->ehdr64.shstrndx;
	}
	reverse_bytes_direction(pentry,	addr_size);
	reverse_bytes_direction(pphoff,	addr_size);
	reverse_bytes_direction(pshoff,	addr_size);
	reverse_bytes_direction(flags,		sizeof(uint));
	reverse_bytes_direction(ehsize,		sizeof(ushort));
	reverse_bytes_direction(phentsize,	sizeof(ushort));
	reverse_bytes_direction(phnum,		sizeof(ushort));
	reverse_bytes_direction(shentsize,	sizeof(ushort));
	reverse_bytes_direction(shnum,		sizeof(ushort));
	reverse_bytes_direction(shstrndx,	sizeof(ushort));

	reverse_bytes_direction(&e->ehdr32.common.type,		sizeof(ushort));
	reverse_bytes_direction(&e->ehdr32.common.machine,	sizeof(ushort));
	reverse_bytes_direction(&e->ehdr32.common.version,	sizeof(uint));
}

bool print_error_msg(const char *const msg) {
	fprintf(stdout, "%s\n", msg);
	return false;
}

// If success, return true and write the maximum fd offset to *ret.
bool get_fd_max_off(int fd, ssize_t *ret) {
	ssize_t fd_curr_off;
	if (!get_fd_cur_off(fd, &fd_curr_off))
		return false;
		
	ssize_t fd_max_off = lseek(fd, 0, SEEK_END);
	if (-1 == fd_max_off)
		return false;
	
	if (-1 == lseek(fd, fd_curr_off, SEEK_SET))
		return false;
	*ret = fd_max_off;
	return true;
}

// If success, return true and write the current fd offset into *ret.
bool get_fd_cur_off(int fd, ssize_t *ret) {
	ssize_t cur_off;
	if (-1 == (cur_off = lseek(fd, 0, SEEK_CUR)))
		return false;
	
	*ret = cur_off;
	return true;
}

// Return true if fd.cur_off + seek_this_much is valid.
bool is_valid_seek(int fd, ssize_t seek_to_here) {
	if (seek_to_here < 0)
		return false;
	
	ssize_t max_off;
	if (!get_fd_max_off(fd, &max_off))
		return false;
	return seek_to_here < max_off;
}

// Safe lseek. Returns true on success.
bool lseek_set_wrap(int fd, off_t offset) {
	if (!is_valid_seek(fd, offset))
		return false;
		
	lseek(fd, offset, SEEK_SET);
	return true;
}

// Read a string from fd into the heap and return a pointer to it.
char *get_str_from(int fd) {
	char *str = malloc(1);
	if (!str) {
		fprintf(stdout, "ERROR: malloc for section header name failed\n");
		return null;
	}
	size_t nchar_read = 0, size_of_str = 1;
	for (;;) {
		if ((size_t)str > ~0ull - nchar_read) {
			fprintf(stdout, "ERROR: imminent integer wraparound\n");
			free(str);
			return null;
		}
		ssize_t ret = read(fd, str + nchar_read++, 1);
		if (ret <= 0) {
			fprintf(stdout, "ERROR: read of section header name failed\n");
			free(str);
			return null;
		}
		if (str[nchar_read - 1] == '\0')
			break;
		if (nchar_read == size_of_str) {
			if (size_of_str > ~0ull / 2) {
				fprintf(stdout,
					"WARN: section header name is too long, truncating\n"
				);
				str[nchar_read - 1] = '\0';
				break;
			}
			size_of_str *= 2;
			str = realloc(str, size_of_str);
			if (!str) {
				fprintf(stdout,
					"ERROR: cannot realloc section header string buffer\n");
				free(str);
				return null;
			}
		} else if (nchar_read > size_of_str) {
			fprintf(stdout, "ERROR: programmer mistake, buffer overflow\n");
			return null;
		}
	}
	
	return str;
}
