// unreusable helper fns

#include "delf_h1.h"
#include "delf_h2.h"

// Print a string according to ei_class. Input must be valid!
void interpret_class(const Elf32_Ehdr_ident *const pehdri) {
	fprintf(stdout, "EI_CLASS:\t");
	const char *classes[] = {
		"ELFCLASSNONE, Invalid class",
		"ELFCLASS32, 32-bit objects",
		"ELFCLASS64, 64-bit objects"
	};
	fprintf(stdout, "%s\n", classes[pehdri->ei_class]);
}

// Print a string according to ei_data. Input must be valid!
void interpret_data(const Elf32_Ehdr_ident *const pehdri) {
	fprintf(stdout, "EI_DATA:\t");
	const char *datas[] = {
		"ELFDATANONE, Invalid data encoding",
		"ELFDATA2LSB, 2's complement values, little-endian",
		"ELFDATA2MSB, 2's complement values, big-endian"
	};
	fprintf(stdout, "%s\n", datas[pehdri->ei_data]);
}

// Print a string according to ei_version. Input must be valid!
void interpret_version(const Elf32_Ehdr_ident *const pehdri) {
	fprintf(stdout, "EI_VERSION:\t%u, Current version\n", pehdri->ei_version);
}

// Print a string according to ei_osabi. Input must be valid!
void interpret_osabi(const Elf32_Ehdr_ident *const pehdri) {
	fprintf(stdout, "EI_OSABI:\t");
	const char *meaning = "N/A";
	if (0 == pehdri->ei_osabi)
		meaning = "ELFOSABI_SYSV, System V ABI";
	else if (1 == pehdri->ei_osabi)
		meaning = "ELFOSABI_HPUX, HP-UX operating system";
	else if (255 == pehdri->ei_osabi)
		meaning = "ELFOSABI_STANDALONE, Standalone (embedded) application";
	fprintf(stdout, "%s\n", meaning);
}

// Print a string according to ei_abiversion. Input must be valid!
void interpret_abiversion(const Elf32_Ehdr_ident *const pehdri) {
	const char *meaning = "N/A";
	if (0 == pehdri->ei_abiversion)
		meaning = "EI_ABIVERSION:\t0, conforming to System V ABI, 3rd ed";
	fprintf(stdout, "%s\n", meaning);
}

// Print padding bytes. Input must be valid!
void print_ehdr_ident_padding(const Elf32_Ehdr_ident *const pehdri) {
	fprintf(stdout, "EI_PAD:\t");
	for (int i = 0; i < 7; i++) {
		int byte = pehdri->ei_pad[i];
		fprintf(stdout, "%.2x ", byte);
	}
	fprintf(stdout, "\n");
}

// Return true if ident is valid. The argument must be a valid pointer.
bool verify_ident(const Elf32_Ehdr_ident *const ident) {
	if (0 != strncmp((const char *const)(ident->ei_magic), "\x7f" "ELF", 4) ||
		ident->ei_class > 2 || ident->ei_data > 2 || ident->ei_version != 1)
		return false;
	return true;
}

// Return true if type is valid.
bool verify_type(const ushort type) {
	if (type > 4 && type < 0xff00)
		return false;
	return true;
}

// Read the "common" field from fd. Input must be valid!
bool read_ge_common(ElfGeneric_Ehdr *ge, int fd) {
	if (!is_valid_seek(fd, sizeof(ElfGeneric_Ehdr_Common)))
		return false;
	return read_into(&ge->ehdr32.common, sizeof(ElfGeneric_Ehdr_Common), fd);
}

// Return true if machine is valid.
bool verify_machine(ushort machine) {
	if (machine == 6 || machine == 9 ||
		(machine > 16 && machine != 62)) {
		return false;
	}
	return true;
}

// Return true if version is valid.
bool verify_version(uint version) {
	if (version != 1)
		return false;
	return true;
}

// Driver fn for verifying some of the ELF header fields.
bool verify_ge_common(ElfGeneric_Ehdr *ge) {
	ElfGeneric_Ehdr_Common *gec = &ge->ehdr32.common;
	if (!verify_ident(&gec->ident) ||
		!verify_type(gec->type) ||
		!verify_machine(gec->machine) ||
		!verify_version(gec->version))
		return false;
	return true;
}

// Stub fn. Currently just returns true.
bool verify_rest_of_ge(const ElfGeneric_Ehdr *const ge) {
	return ge + 0 == ge;
}

// Driver fn. Processes an ELF header completely.
bool read_elf_header(ElfGeneric_Ehdr *ge, int fd) {
	// read and verify the first part of the ELF header
	if (!read_ge_common(ge, fd) || !verify_ge_common(ge))
		return false;
	
	size_t bytes_toread = sizeof(Elf32_Ehdr) - sizeof(ElfGeneric_Ehdr_Common);
	if (is_Elf64(ge))
		bytes_toread = sizeof(Elf64_Ehdr) - sizeof(ElfGeneric_Ehdr_Common);
	// read and verify the rest of the ELF header
	read_into((void *)((const char *)ge + sizeof(ge->ehdr32.common)),
		bytes_toread, fd);
	verify_rest_of_ge(ge);
	
	maybe_reverse_e_endianness(ge);
	return true;
}

// ???
bool print_entry_generic(const void *const u, size_t sz, const uchar direction,
	const char *const nameofentry) {
	fprintf(stdout, "%s:\t", nameofentry);
	
	if (direction == ELFDATANONE)
		fprintf(stdout, "N/A");
	else
		print_sz_bytes_le(u, sz);
	fprintf(stdout, "\n");
	return true;
}

// Print usage information on standard sections (which are identified by name).
bool print_standard_section_use(const char *const sh_name) {
	const char *msg = null;
	if (0 == strcmp(sh_name,		".bss"))
		msg = "Uninitialized data";
	else if (0 == strcmp(sh_name,	".data"))
		msg = "Initialized data";
	else if (0 == strcmp(sh_name, 	".interp"))
		msg = "Program interpreter path name";
	else if (0 == strcmp(sh_name, 	".rodata"))
		msg = "Read-only data (constants and literals)";
	else if (0 == strcmp(sh_name, 	".text"))
		msg = "Executable code";
	else if (0 == strcmp(sh_name, 	".comment"))
		msg = "Version control information";
	else if (0 == strcmp(sh_name, 	".dynamic"))
		msg = "Dynamic linking tables";
	else if (0 == strcmp(sh_name, 	".dynstr"))
		msg = "String table for .dynamic section";
	else if (0 == strcmp(sh_name, 	".dynsym"))
		msg = "Symbol table for dynamic linking";
	else if (0 == strcmp(sh_name, 	".got"))
		msg = "Global offset table";
	else if (0 == strcmp(sh_name, 	".hash"))
		msg = "Symbol hash table";
	else if (0 == strcmp(sh_name, 	".note"))
		msg = "Note section";
	else if (0 == strcmp(sh_name, 	".plt"))
		msg = "Procedure linkage table";
	else if (0 == strncmp(sh_name, 	".rel", 4))  // !
		msg = "Relocations for section";
	else if (0 == strcmp(sh_name, 	".shstrtab"))
		msg = "Section name string table";
	else if (0 == strcmp(sh_name, 	".strtab"))
		msg = "String table";
	else if (0 == strcmp(sh_name, 	".symtab"))
		msg = "Linker symbol table";
	
	if (!msg)
		return true;
	
	if (0 > fprintf(stdout, ", %s", msg))
		return false;
	return true;
}

bool print_section_name (const void *const v, size_t vsz, int fd, 
	ull nstsf, ushort i) {
	if (nstsf > ~0ull - ((Elf32_Shdr *)v)->name)
		return print_error_msg("imminent integer overflow");
	ull offset_to_sh_name = nstsf + ((Elf32_Shdr *)v)->name;
	
	if (vsz == sizeof(Elf64_Shdr)) {
		if (nstsf > ~0ull - ((Elf64_Shdr *)v)->name)
			return print_error_msg("imminent integer overflow");
		offset_to_sh_name = nstsf + ((Elf64_Shdr *)v)->name;
	}
	
	if (!lseek_set_wrap(fd, offset_to_sh_name))
		return print_error_msg("lseek to section header name failed");
	char *sh_name = get_str_from(fd);
	fprintf(stdout, "SECTION HEADER %.2d\n", i);
	fprintf(stdout, "sh_name:\t%s", sh_name);
	if (!print_standard_section_use(sh_name))
		return false;
	fprintf(stdout, "\n");
	free(sh_name);
	return true;
}

bool print_section_type(const void *const v, const size_t vsz) {
	uint shtype = ((ElfGeneric_Shdr *)v)->eshdr32.sh_type;
	if (vsz == sizeof(Elf64_Shdr))
		shtype = ((ElfGeneric_Shdr *)v)->eshdr64.type;
	
	fprintf(stdout, "sh_type:\t");
	const char *const section_types[] = {
		"SHT_NULL, Marks an unused section header",
		"SHT_PROGBITS, Contains information defined by the program",
		"SHT_SYMTAB, Contains a linker symbol table",
		"SHT_STRTAB, Contains a string table",
		"SHT_RELA, Contains \"Rela\" type relocation entries",
		"SHT_HASH, Contains a symbol hash table",
		"SHT_DYNAMIC, Contains dynamic linking tables",
		"SHT_NOTE, Contains note information",
		"SHT_NOBITS, Contains uninitialized space; " \
			"does not occupy any space in the file",
		"SHT_REL, Contains \"Rel\" type relocation entries",
		"SHT_SHLIB, Reserved",
		"SHT_DYNSYM, Contains a dynamic loader symbol table"
	};
	if (shtype <= 11)
		fprintf(stdout, "%s", section_types[shtype]);
	else if (shtype > 0x60000000 && shtype < 0x6fffffff)
		fprintf(stdout, "SHT_LOOS-SHT_HIOS, Environment-specific use");
	else if (shtype >70000000 && shtype < 0x7fffffff)
		fprintf(stdout, "SHT_LOPROC-SHT_HIPROC, Processor-specific use");
	else
		fprintf(stdout, "N/A");
	fprintf(stdout, "\n");
	
	return true;
}

bool print_section_flags(const void *const v, size_t vsz) {
	ull flags = ((ElfGeneric_Shdr *)v)->eshdr32.flags;
	if (vsz == sizeof(Elf64_Shdr))
		flags = ((ElfGeneric_Shdr *)v)->eshdr64.flags;
	
	fprintf(stdout, "sh_flags:\tBelow are the flags of this section, if any\n");
	if (0x1 & flags)
		fprintf(stdout, "\t\tSHF_WRITE, Section contains writable data\n");
	if (0x2 & flags)
		fprintf(stdout, "\t\tSHF_ALLOC, Section is allocated in memory image" \
			" of program\n");
	if (0x4 & flags)
		fprintf(stdout, "\t\tSHF_EXECINSTR, Section contains executable" \
			" instructions\n");
	if (0x0F000000 & flags)
		fprintf(stdout, "\t\tSHF_MASKOS, Environment-specific use\n");
	if (0xF0000000 & flags)
		fprintf(stdout, "\t\tSHF_MASKPROC, Processor-specific use\n");
	
	return true;
}

// Print "member: 0xnumber \n".
bool print_hex_16_digits(const char *const member, ull number) {
	if (0 > fprintf(stdout, "%s:\t0x%.16llx\n", member, number))
		return false;
	return true;
}

bool print_section_addr(const void *const v, size_t vsz) {
	ull addr = ((ElfGeneric_Shdr *)v)->eshdr32.addr;
	if (vsz == sizeof(Elf64_Shdr))
		addr = ((ElfGeneric_Shdr *)v)->eshdr64.addr;
	
	print_hex_16_digits("sh_addr", addr);
	return true;
}

bool print_section_offset(const void *const v, size_t vsz) {
	ull offset = ((ElfGeneric_Shdr *)v)->eshdr32.offset;
	if (vsz == sizeof(Elf64_Shdr))
		offset = ((ElfGeneric_Shdr *)v)->eshdr64.off;
	
	print_hex_16_digits("sh_offset", offset);
	return true;
}

bool print_section_size(const void *const v, size_t vsz) {
	ull size = ((ElfGeneric_Shdr *)v)->eshdr32.size;
	if (vsz == sizeof(Elf64_Shdr))
		size = ((ElfGeneric_Shdr *)v)->eshdr64.size;
	
	print_hex_16_digits("sh_size", size);
	return true;
} 

char *get_section_name(ull shoff, const void *const v, size_t vsz, int fd,
	ull nstsf) {
	if (vsz == sizeof(Elf32_Shdr)) {
		Elf32_Shdr *e = &((ElfGeneric_Shdr *)v)->eshdr32;
		if (!lseek_set_wrap(fd, shoff + sizeof(Elf32_Shdr) * e->link))
			return null;
		Elf32_Shdr assoc_shdr;
		if (!read_into(&assoc_shdr, sizeof(assoc_shdr), fd))
			return null;
		if (!lseek_set_wrap(fd, nstsf + assoc_shdr.name))
			return null;
		return get_str_from(fd);
	} else {  // 64-bit ELF file
		Elf64_Shdr *e = &((ElfGeneric_Shdr *)v)->eshdr64;
		if (!lseek_set_wrap(fd, shoff + sizeof(Elf64_Shdr) * e->link))
			return null;  // failed
		Elf64_Shdr assoc_shdr;
		if (!read_into(&assoc_shdr, sizeof(assoc_shdr), fd))
			return null;  // failed
		if (!lseek_set_wrap(fd, nstsf + assoc_shdr.name))
			return null;
		return get_str_from(fd);
	}
	return null;
}

bool print_associated_sect_name(ull shoff, const void *const v, size_t vsz,
	int fd, ull nstsf) {
	char *associated_sect_name = get_section_name(shoff, v, vsz, fd, nstsf);
	if (!associated_sect_name)
		return false;
	
	bool ret = true;
	if (0 > fprintf(stdout, "\n\t\tAssociated section is a \"%s\". Member "\
		"value is ", associated_sect_name))
		ret = false;
	free(associated_sect_name);
	return ret;
}

bool print_associated_sect_meaning(const void *const v, size_t vsz) {
	const char *msg = "SHN_UNDEF";
	uint type = ((ElfGeneric_Shdr *)v)->eshdr32.sh_type;
	if (vsz == sizeof(Elf64_Shdr))
		type = ((ElfGeneric_Shdr *)v)->eshdr64.type;

	if (type == SHT_DYNAMIC)
		msg = "the section header index of the string table used by entries "\
			"in the section.";
	else if (type == SHT_HASH)
		msg = "the section header index of the symbol table to which the hash" \
			"table applies.";
	else if (type == SHT_REL || type == SHT_RELA)
		msg = "the section header index of the associated symbol table.";
	else if (type == SHT_SYMTAB || type == SHT_DYNSYM) {
		if (vsz == sizeof(Elf32_Shdr))
			msg = "OS-specific information.";
		else  // 64-bit
			msg = "the section header index of the string table used by "\
				"entries in this section";
	}
	
	if (0 > fprintf(stdout, "%s\n", msg))
		return false;
	return true;
}

bool print_section_link(ull shoff, const void *const v, size_t vsz, int fd,
	ull nstsf) {
	uint link = ((ElfGeneric_Shdr *)v)->eshdr32.link;
	if (vsz == sizeof(Elf64_Shdr))
		link = ((ElfGeneric_Shdr *)v)->eshdr64.link;
	
	if (0 > fprintf(stdout, "sh_link:\t0x%.16x", link))
		return false;
	if (!link) {
		if (0 > fprintf(stdout, "\n"))
			return false;
		return true;
	}
	
	if (!print_associated_sect_name(shoff, v, vsz, fd, nstsf))
		return false;
	
	if (!print_associated_sect_meaning(v, vsz))
		return false;
	return true;
}

// Tin. Inputs must be valid!
bool print_section_info(const void *const v, size_t vsz) {
	uint info, type;
	const char *msg = "";
	if (vsz == sizeof(Elf32_Shdr)) {
		info = ((ElfGeneric_Shdr *)v)->eshdr32.info;
		type = ((ElfGeneric_Shdr *)v)->eshdr32.sh_type;
		const char *msg1 = ", The section header index of the section to " \
			"which the relocation applies.";
		const char *msg2 = "This information is operating system specific.";
		if (type == SHT_REL || type == SHT_RELA)
			msg = msg1;
		else if (type == SHT_SYMTAB || type == SHT_DYNSYM)
			msg = msg2;
	} else { // Elf64_Shdr
		info = ((ElfGeneric_Shdr *)v)->eshdr64.info;
		type = ((ElfGeneric_Shdr *)v)->eshdr64.type;
		const char *msg1 = ", Section index of section to which the " \
			"relocations apply";
		const char *msg2 = ", Index of first non-local symbol (i.e., number " \
			"of local symbols)";
		if (type == SHT_REL || type == SHT_RELA)
			msg = msg1;
		else if (type == SHT_SYMTAB || type == SHT_DYNSYM)
			msg = msg2;
	}
	if (0 > fprintf(stdout, "sh_info:\t%u%s\n", info, msg))
		return false;
	return true;
}

// Tin. Inputs must be valid!
bool print_section_addralign(const void *const v, size_t vsz) {
	ull addralign;
	if (vsz == sizeof(Elf32_Shdr))
		addralign = ((ElfGeneric_Shdr *)v)->eshdr32.addralign;
	else
		addralign = ((ElfGeneric_Shdr *)v)->eshdr32.addralign;
	if (0 > fprintf(stdout, "sh_addralign:\t%.2llu\n", addralign))
		return false;
	return true;
}

bool print_section_entsize(const void *const v, size_t vsz) {
	ull entsize;
	if (vsz == sizeof(Elf32_Shdr))
		entsize = ((ElfGeneric_Shdr *)v)->eshdr32.entsize;
	else
		entsize = ((ElfGeneric_Shdr *)v)->eshdr64.entsize;
	if (0 > fprintf(stdout, "sh_entsize:\t%.2llu\n", entsize))
		return false;
	return true;
}

// Tin. Input must be valid!
bool print_section_header_details(ull shoff, const void *const v, size_t vsz, 
	int fd, ull nstsf, ushort i) {
	return
		print_section_name(v, vsz, fd, nstsf, i) &&
		print_section_type(v, vsz) &&
		print_section_flags(v, vsz) &&
		print_section_addr(v, vsz) &&
		print_section_offset(v, vsz) &&
		print_section_size(v, vsz) &&
		print_section_link(shoff, v, vsz, fd, nstsf) &&
		print_section_info(v, vsz) &&
		print_section_addralign(v, vsz) &&
		print_section_entsize(v, vsz);
}

// Tin. Input must be valid!
bool get_a_section_header(void *const v, size_t vsz, int fd,
	ull shoff, ushort i) {
	if (!lseek_set_wrap(fd, shoff + i * vsz))
		return false;
	if (!read_into(v, vsz, fd))
		return false;
	return true;
}

bool print_section_strings_maybe_32(const Elf32_Shdr *const es, int fd) {
	if (es->sh_type != SHT_STRTAB)
		return true;
	
	fprintf(stdout, "section strings:\n");
	ull num_chars_read = 0;
	if (!lseek_set_wrap(fd, es->offset))
		return false;
	while (num_chars_read < es->size) {
		char *const str = get_str_from(fd);
		if (0 == strlen(str))
			fprintf(stdout, "\t(null)\n");
		else
			fprintf(stdout, "\t%s\n", str);
		num_chars_read += strlen(str) + 1;  // don't worry it won't overflow
		if (!lseek_set_wrap(fd, es->offset + num_chars_read))
			return false;
		free(str);
	}
	return true;
}

bool print_section_strings_maybe_64(const Elf64_Shdr *const es, int fd) {
	if (es->type != SHT_STRTAB)
		return true;
	
	fprintf(stdout, "section strings:\n");
	ull num_chars_read = 0;
	if (!lseek_set_wrap(fd, es->off))
		return false;
	while (num_chars_read < es->size) {
		char *const str = get_str_from(fd);
		if (0 == strlen(str))
			fprintf(stdout, "\t(null)\n");
		else
			fprintf(stdout, "\t%s\n", str);
		num_chars_read += strlen(str) + 1;  // don't worry it won't overflow
		if (!lseek_set_wrap(fd, es->off + num_chars_read))
			return false;
		free(str);
	}
	return true;
}

bool print_section_strings_maybe(const void *const v, int fd, bool is_64_bit) {
	if (!is_64_bit) {
		const Elf32_Shdr *const es = (const Elf32_Shdr *const)v;
		return print_section_strings_maybe_32(es, fd);
	} else {
		const Elf64_Shdr *const es = (const Elf64_Shdr *const)v;
		return print_section_strings_maybe_64(es, fd);
	}
	return true;
}
