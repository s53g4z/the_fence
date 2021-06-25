// "direct"-ish fns

#include "delf_h1.h"
#include "delf_h2.h"

bool readelf_ident(ElfGeneric_Ehdr *ge) {
	Elf32_Ehdr_ident *pehdri = &ge->ehdr32.common.ident;
	interpret_class(pehdri);
	interpret_data(pehdri);
	interpret_version(pehdri);
	interpret_osabi(pehdri);
	interpret_abiversion(pehdri);
	
	print_ehdr_ident_padding(pehdri);
	return true;
}

bool readelf_type(const ElfGeneric_Ehdr *const ge) {
	ushort type = ge->ehdr32.common.type;
	fprintf(stdout, "e_type:\t");
	
	const char *types[] = {
		"ET_NONE, No file type",
		"ET_REL, Relocatable file",
		"ET_EXEC, Executable file",
		"ET_DYN, Shared object file",
		"ET_CORE, Core file",
		"ET_**PROC, Processor-specific"
	};
	if (type > 4)
		fprintf(stdout, "%s\n", types[sizeof(types)/sizeof(const char *) - 1]);
	else
		fprintf(stdout, "%s\n", types[type]);
	return true;
}

bool readelf_machine(const ElfGeneric_Ehdr *const ge) {
	ushort machine = ge->ehdr32.common.machine;
	fprintf(stdout, "e_machine:\t");

	const char *machines[] = {
		"ET_NONE, No machine",
		"EM_M32, AT&T WE 32100",
		"EM_SPARC, SPARC",
		"EM_386, Intel Architecture",
		"EM_68K, Motorola 68000",
		"EM_88K, Motorola 88000",
		"",
		"EM_860, Intel 80860",
		"EM_MIPS, MIPS RS3000 Big-Endian",
		"",
		"EM_MIPS_RS4_BE, MIPS RS4000 Big-Endian",
		"RESERVED, Reserved for future use",
		"EM_X86_64, AMD64"
	};
	
	int machines_offset = machine;
	if (machine == 62)
		machines_offset = sizeof(machines)/sizeof(const char *) - 1;
	else if (machine > 10)
		machines_offset = 11;
	fprintf(stdout, "%s\n", machines[machines_offset]);
	
	return true;
}

bool readelf_version(const ElfGeneric_Ehdr *const ge) {
	uint version = ge->ehdr32.common.version;
	fprintf(stdout, "e_version:\t");
	
	const char *versions[] = {
		"EV_NONE, Invalid versionn",  // (sic)
		"EV_CURRENT, Current version"
	};
	fprintf(stdout, "%s\n", versions[version]);
	return true;
}

bool readelf_entry(const ElfGeneric_Ehdr *const ge) {
	const void *pentry = &ge->ehdr32.entry;
	if (is_Elf64(ge))
		pentry = &ge->ehdr64.entry;
	return print_entry_generic(pentry,
		is_Elf64(ge) ? sizeof(ull) : sizeof(uint),
		ge->ehdr32.common.ident.ei_data, "e_entry");
}

bool readelf_phoff(const ElfGeneric_Ehdr *const ge) {
	const void *pphoff = &ge->ehdr32.phoff;
	if (is_Elf64(ge))
		pphoff = &ge->ehdr64.phoff;
	return print_entry_generic(pphoff,
		is_Elf64(ge) ? sizeof(ull) : sizeof(uint),
		ge->ehdr32.common.ident.ei_data, "e_phoff");
}

bool readelf_shoff(const ElfGeneric_Ehdr *const ge) {
	const void *pshoff = &ge->ehdr32.shoff;
	if (is_Elf64(ge))
		pshoff = &ge->ehdr64.shoff;
	return print_entry_generic(pshoff,
		is_Elf64(ge) ? sizeof(ull) : sizeof(uint),
		ge->ehdr32.common.ident.ei_data, "e_shoff");
}

bool readelf_flags(const ElfGeneric_Ehdr *const ge) {
	const uint *flags = &ge->ehdr32.flags;
	if (is_Elf64(ge))
		flags = &ge->ehdr64.flags;
	return print_entry_generic(flags, sizeof(uint),
		ge->ehdr32.common.ident.ei_data, "e_flags");
}

bool readelf_ehsize(const ElfGeneric_Ehdr *const ge) {
	const ushort *ehsize = &ge->ehdr32.ehsize;
	if (is_Elf64(ge))
		ehsize = &ge->ehdr64.ehsize;
	fprintf(stdout, "e_ehsize:\t%hu\n", *ehsize);
	return true;
}

bool readelf_phentsize(const ElfGeneric_Ehdr *const ge) {
	const ushort *phentsize = &ge->ehdr32.phentsize;
	if (is_Elf64(ge))
		phentsize = &ge->ehdr64.phentsize;
	fprintf(stdout, "e_phentsize:\t%hu\n", *phentsize);
	return true;
}

bool readelf_phnum(const ElfGeneric_Ehdr *const ge) {
	const ushort *phnum = &ge->ehdr32.phnum;
	if (is_Elf64(ge))
		phnum = &ge->ehdr64.phnum;
	fprintf(stdout, "e_phnum:\t%hu\n", *phnum);
	return true;
}

bool readelf_shentsize(const ElfGeneric_Ehdr *const ge) {
	const ushort *shentsize = &ge->ehdr32.shentsize;
	if (is_Elf64(ge))
		shentsize = &ge->ehdr64.shentsize;
	fprintf(stdout, "e_shentsize:\t%hu\n", *shentsize);
	return true;
}

bool readelf_shnum(const ElfGeneric_Ehdr *const ge) {
	const ushort *shnum = &ge->ehdr32.shnum;
	if (is_Elf64(ge))
		shnum = &ge->ehdr64.shnum;
	fprintf(stdout, "e_shnum:\t%hu\n", *shnum);
	return true;
}

bool readelf_shstrndx(const ElfGeneric_Ehdr *const ge) {
	const ushort *shstrndx = &ge->ehdr32.shstrndx;
	if (is_Elf64(ge))
		shstrndx = &ge->ehdr64.shstrndx;
	fprintf(stdout, "e_shstrndx:\t%hu\n", *shstrndx);
	return true;
}

bool print_elf_header(ElfGeneric_Ehdr *ge) {
	return
		readelf_ident(ge) &&
		readelf_type(ge) &&
		readelf_machine(ge) &&
		readelf_version(ge) &&
		readelf_entry(ge) &&
		readelf_phoff(ge) &&
		readelf_shoff(ge) &&
		readelf_flags(ge) &&
		readelf_ehsize(ge) &&
		readelf_phentsize(ge) &&
		readelf_phnum(ge) &&
		readelf_shentsize(ge) &&
		readelf_shnum(ge) &&
		readelf_shstrndx(ge);
}

// Print the sections in an ELF file. Inputs must be valid!
bool print_sections(ElfGeneric_Ehdr *pge, int fd) {
	ushort nsections = pge->ehdr32.shnum;
	ull shoff = pge->ehdr32.shoff;  // SHT offset
	ull shstrndx = pge->ehdr32.shstrndx;  // index into SH array
	if (is_Elf64(pge)) {
		nsections = pge->ehdr64.shnum;
		shoff = pge->ehdr64.shoff;
		shstrndx = pge->ehdr64.shstrndx;
	}
	ull nst_hdr_offset_fdoffset = shoff + shstrndx *
		(is_Elf64(pge) ? sizeof(Elf64_Shdr) : sizeof(Elf32_Shdr)) +
		(is_Elf64(pge) ?
		offsetof(Elf64_Shdr, off) : offsetof(Elf32_Shdr, offset));
	if (!lseek_set_wrap(fd, nst_hdr_offset_fdoffset))
		return print_error_msg("ERROR: lseek to name str table hdr failed");
	uint name_string_table_sect_fileoffset;  // important var
	if (!read_into(&name_string_table_sect_fileoffset, sizeof(uint), fd))
		return print_error_msg("ERROR: read of name str table offset failed");
	
	const char *const sh_read_fail = "read of section header in arr failed";
	const char *const sh_print_fail = "print of section header in arr failed";
	
	if (!is_Elf64(pge)) {
		for (ushort i = 0; i < nsections; i++) {
			// get a section header
			Elf32_Shdr e32shdr;
			if (!get_a_section_header(&e32shdr, sizeof(e32shdr), fd, shoff, i))
				return print_error_msg(sh_read_fail);
			
			// print section header
			if (!print_section_header_details(shoff, &e32shdr, sizeof(e32shdr),
				fd, name_string_table_sect_fileoffset, i) ||
				!print_section_strings_maybe(&e32shdr, fd, is_Elf64(pge)))
				return print_error_msg(sh_print_fail);
		}
	} else {
		for (ushort i = 0; i < nsections; i++) {
			// get a section header
			Elf64_Shdr e64shdr;
			if (!get_a_section_header(&e64shdr, sizeof(e64shdr), fd, shoff, i))
				return print_error_msg(sh_read_fail);
			
			// print section header
			if (!print_section_header_details(shoff, &e64shdr, sizeof(e64shdr),
				fd, name_string_table_sect_fileoffset, i) ||
				!print_section_strings_maybe(&e64shdr, fd, is_Elf64(pge)))
				return print_error_msg(sh_print_fail);
		}
	}
	
	return true;
}
