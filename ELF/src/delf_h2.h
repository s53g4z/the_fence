#ifndef TWO_H
#define TWO_H

#if true  // for geany code folding
bool print_err_not_elf_file(const char *const filename);
bool host_is_le(void);
void interpret_class(const Elf32_Ehdr_ident *pehdri);
void interpret_data(const Elf32_Ehdr_ident *const pehdri);
void interpret_version(const Elf32_Ehdr_ident *const pehdri);
void print_ehdr_ident_padding(const Elf32_Ehdr_ident *const pehdri);
bool verify_ident(const Elf32_Ehdr_ident *const ident);
bool readelf_ident(ElfGeneric_Ehdr *ge);
bool verify_type(const ushort type);
bool readelf_type(const ElfGeneric_Ehdr *const e);
void reverse_bytes_direction(void *const bytes, const size_t howmany);
void maybe_reverse_e_endianness(ElfGeneric_Ehdr *const e);
bool read_into(void *bytes, ssize_t nbytes, int fd);
bool read_ge_common(ElfGeneric_Ehdr *ge, int fd);
bool verify_machine(ushort machine);
bool verify_version(uint version);
bool verify_ge_common(ElfGeneric_Ehdr *ge);
bool verify_rest_of_ge(const ElfGeneric_Ehdr *const ge);
bool read_elf_header(ElfGeneric_Ehdr *ge, int fd);
bool readelf_machine(const ElfGeneric_Ehdr *const e);
bool readelf_version(const ElfGeneric_Ehdr *const e);
void print_byte_as_hex(const uchar byte);
void print_sz_bytes_le(const char *data, uint size);
bool print_entry_generic(const void *const u, size_t sz, const uchar direction,
	const char *const nameofentry);
bool is_Elf64(const ElfGeneric_Ehdr *const e);
bool readelf_entry(const ElfGeneric_Ehdr *const e);
bool readelf_phoff(const ElfGeneric_Ehdr *const e);
bool readelf_shoff(const ElfGeneric_Ehdr *const e);
bool readelf_flags(const ElfGeneric_Ehdr *const e);
bool readelf_ehsize(const ElfGeneric_Ehdr *const e);
bool readelf_phentsize(const ElfGeneric_Ehdr *const e);
bool readelf_phnum(const ElfGeneric_Ehdr *const e);
bool readelf_shentsize(const ElfGeneric_Ehdr *const e);
bool readelf_shnum(const ElfGeneric_Ehdr *const e);
bool readelf_shstrndx(const ElfGeneric_Ehdr *const e);
bool readelf(const char *const filename);
bool print_elf_header(ElfGeneric_Ehdr *ge);
void interpret_osabi(const Elf32_Ehdr_ident *const pehdri);
void interpret_abiversion(const Elf32_Ehdr_ident *const pehdri);
#endif

bool print_sections(ElfGeneric_Ehdr *pge, int fd);
bool print_error_msg(const char *const msg);
bool lseek_set_wrap(int fd, off_t offset);
bool get_fd_max_off(int fd, off_t *ret);
bool get_fd_cur_off(int fd, off_t *ret);
bool is_valid_seek(int fd, ssize_t seek_this_much);
char *get_str_from(int fd);
bool get_a_section_header(void *const v, size_t vsz, int fd,
	ull shoff, ushort i);
bool print_section_header_details(ull, const void *const v, size_t vsz, int fd, 
	ull nstsf, ushort i);
bool print_section_strings_maybe(const void *const v, int fd, bool is_64_bit);

#endif
