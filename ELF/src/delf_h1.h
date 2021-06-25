#ifndef ONE_H
#define ONE_H

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stddef.h>

_Static_assert (
	CHAR_BIT == 8 &&
	sizeof(short) == 2 &&
	sizeof(int) == 4 &&
	sizeof(long long) == 8 &&
	sizeof(size_t) == 8 &&
	sizeof(ssize_t) == 8 &&
	sizeof(off_t) == 8,
	"architecture must be recognized"
);

// none, relocatable, executable, shared obj, core (dump)
enum tyype { NONE, REL, EXEC, DYN, CORE };
// anything inclusively in-between LOPROC and HIGHPROC have CPU-specific meaning
#define LOPROC 0xff00
#define HIPROC 0xffff
_Static_assert(NONE == 0, "bad enum");

// AT&T, SPARC, IA-32, MROLA, MROLA, 80860, MIPS, MIPS, (11-16 are reserved)
enum maachine { ATT32 = 1, SPARC, I386, M68K, M88K, I860 = 7,
	MIPS, MIPS_RS4 = 10 };
_Static_assert(M88K == 5 && MIPS == 8 && MIPS_RS4 == 10);

// MAG? for identification, DATA for data encoding
enum ident_byte_indexes { MAG0, MAG1, MAG2, MAG3, CLASS, DATA, VERSION, PAD };
// Note: I386 must have ELFCLASS32 and ELFDATA2LSB.
enum ident_byte_values {
	ELFMAG0 = 0x7f,
	ELFMAG1 = 'E', ELFMAG2 = 'L', ELFMAG3 = 'F',
	ELFCLASSNONE = 0, ELFCLASS32, ELFCLASS64,  // 0 == invalid class
	ELFDATANONE = 0, ELFDATA2LSB, ELFDATA2MSB  // 0 == invalid data encoding
};

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned uint;
typedef unsigned long long ull;
typedef uchar bool;

#define true 1
#define false 0
#define null 0

typedef struct {
	uchar ei_magic[4];  // = '\x7f', 'E', 'L', 'F';
	uchar ei_class;
	uchar ei_data;
	uchar ei_version;
	uchar ei_osabi;  // new!
	uchar ei_abiversion;  // new!
	uchar ei_pad[7];
} Elf32_Ehdr_ident;

_Static_assert(sizeof(Elf32_Ehdr_ident) == 16,
	"Elf32_Ehdr_ident can't have gaps");

typedef struct {
	Elf32_Ehdr_ident ident;	// file decode instructions
	ushort type;		// file type
	ushort machine;		// file architecture
	uint version;		// == 1
} ElfGeneric_Ehdr_Common;

typedef struct {
	ElfGeneric_Ehdr_Common common;
	uint entry;			// == 0 || address of _start
	uint phoff;			// == 0 || PHT offset, (program header table)
	uint shoff;			// == 0 || SHT offset, (section header table)
	uint flags;			// CPU-specific. == 0 for I386.
	ushort ehsize;		// size of ELF header
	ushort phentsize;	// size of a PHT entry
	ushort phnum;		// number of PHT entries
	ushort shentsize;	// size of SHT entry, (section header entry)
	ushort shnum;		// number of SHT entries
	ushort shstrndx;	// index to "section name string table" entry in SHT
} Elf32_Ehdr;

_Static_assert(sizeof(Elf32_Ehdr) ==
	16 + sizeof(ushort) * 8 + sizeof(uint) * 5, "Elf32_Ehdr can't have gaps");

typedef struct {
	ElfGeneric_Ehdr_Common common;
	ull entry;
	ull phoff;
	ull shoff;
	uint flags;
	ushort ehsize;	// size of ELF header
	ushort phentsize;	// size of a PHT entry
	ushort phnum;		// number of PHT entries
	ushort shentsize;	// size of SHT entry, (section header entry)
	ushort shnum;		// number of SHT entries
	ushort shstrndx;	// index to "section name string table" entry in SHT
} Elf64_Ehdr;

_Static_assert(sizeof(Elf64_Ehdr) ==
	sizeof(ElfGeneric_Ehdr_Common) + sizeof(ull) * 3 + sizeof(uint) +
	sizeof(ushort) * 6, "Elf64_Ehdr can't have gaps");

typedef union {
	Elf32_Ehdr ehdr32;
	Elf64_Ehdr ehdr64;
} ElfGeneric_Ehdr;

enum section_indices_generic {
	SHN_UNDEF,
	SHN_LOPROC = 0xff00, SHN_HIPROC = 0xff1f,
	SHN_ABS = 0xfff1, SHN_COMMON, SHN_HIRESERVE = 0xffff
};

enum section_indices_32 {
	SHN_LORESERVE = 0xff00,
};

enum section_indices_64 {
	SHN_LOOS = 0xff20, SHN_HIOS=0xff3f
};

// inactive?, undefined, holds symtab, holds strtab, ???, holds symhashtab,
// dynamic link info, undefined, undefined, ???, reserved, clone of SHT_SYMTAB
enum sh_type {
	SHT_NULL, SHT_PROGBITS, SHT_SYMTAB, SHT_STRTAB, SHT_RELA, SHT_HASH,
	SHT_DYNAMIC, SHT_NOTE, SHT_NOBITS, SHT_REL, SHT_SHLIB, SHT_DYNSYM,
	SHT_LOPROC = 0x70000000, SHT_HIPROC = 0x7fffffff,
	SHT_LOUSER = 0x80000000, SHT_HIUSER= 0xffffffff
};
// Note: Dynamic linking information for I386 is in .dynsym, .dynstr, .interp,
//	.hash, .dynamic, .rel, .rela, .got, and .plt.
// Note: .init and .fini "contribute to process ini[t] and termination code".
//	Also see page 67.

// writable, mapped to memory, contains CPU instructions, CPU-specific bits
enum sect_attrib_flags {
	SHF_WRITE = 0x1, SHF_ALLOC = 0x2, SHF_EXECINSTR = 0x4,
	SHF_MASKPROC = 0xf0000000
};

// sh_link interpretation:
// if sh_type == SHT_DYNAMIC
//		link = "The sxtion hdr idx of the strtab used by this sxtion's entries"
//		info = 0
// elif sh_type == SHT_HASH
//		link = "The sxtion hdr idx of the symtab to which this hashtab applies"
//		info = 0
// elif sh_type == SHT_REL(A)
//		link = "The sxtion hdr idx of the associated symtab"
//		info = "The sxtion hdr idx of the sxtion to which the reloc. applies"
// elif sh_type == SYMTAB || sh_type == SHT_DYNSYM
//		link = OS-specific data
//		info = OS-specific data
// else
//		link = SHN_UNDEF
//		info = 0

typedef struct {
	uint name;		// pointer to a null-terminated string in the strtab
	uint sh_type;	// semantics
	uint flags;		// see "sect_attrib_flags", attributes
	uint addr;		// == 0 || memory address to put the section in
	uint offset;	// offset into the file to the section
	uint size;		// size of the section
	uint link;		// see page 66, "sh_link interpretation"
	uint info;
	uint addralign;	// == 0 || declares the section alignment constraint
	uint entsize;	// == 0 || size of a section's fixed-size entry
} Elf32_Shdr;

typedef struct {
	uint name;
	uint type;
	ull flags;
	ull addr;
	ull off;
	ull size;
	uint link;
	uint info;
	ull addralign;
	ull entsize;
} Elf64_Shdr;

typedef union {
	Elf32_Shdr eshdr32;
	Elf64_Shdr eshdr64;
} ElfGeneric_Shdr;

_Static_assert(sizeof(Elf32_Shdr) == sizeof(uint) * 10,
	"Elf32_Shdr can't have gaps");

//Elf32_Shdr SHT_entry0 = { 0 };
// note: page 29 of elf.pdf has a chart on special sections' sh_type and flags


enum sym_binding { STB_LOCAL,
	STB_GLOBAL, STB_WEAK,  // weak == "lower precedence" global
	STB_LOPROC = 13, STB_HIPROC = 15
};

enum sym_types { STT_NOTYPE, STT_OBJECT, STT_FUNC, STT_SECTION, STT_FILE,
	STT_LOPROC = 13, STT_HIPROC = 15
};

typedef struct {
	uint name;		// index into "symbol string table"
	uint value;		// see page 35.
	uint size;		// data payload
	uchar info;		// "symbo[l] type and binding attributes"
	uchar other;	// == 0
	ushort shndx;	// related SHT index
} Elf32_Sym;

_Static_assert(sizeof(Elf32_Sym) == sizeof(uint) * 3 + 2 + sizeof(ushort),
	"Elf32_Sym can't have gaps");

//Elf32_Sym symbol_table_entry_0 = { 0 };


enum relocation_types {
	R386_NONE, R38632, R386PC32
};

typedef struct {
	uint offset;	// if reloc, then is the offset from beginning of sect.
					// else, is the victim's virtual addr.
	uint info;		// victim's symbol table index
} Elf32_Rel;

_Static_assert(sizeof(Elf32_Rel) == sizeof(uint) * 2,
	"Elf32_Rel can't have gaps");

typedef struct {
	uint offset;
	uint info;
	int addend;
} Elf32_Rela;

_Static_assert(sizeof(Elf32_Rela) == sizeof(uint) * 2 + sizeof(int),
	"Elf32_Rela can't have gaps");

// see page 41 for definitions
enum segment_types {
	PT_NULL, PT_LOAD, PT_DYNAMIC, PT_INTERP, PT_NOTE, PT_SHLIB, PT_PHDR,
	PT_LOPROC = 0x70000000, PT_HIPROC = 0x7fffffff
};

// A segment contains section(s). A program header describes a segment.
typedef struct {
	uint ptype;
	uint offset;
	uint vaddr;		// segment goes here in memory
	uint paddr;
	uint filesz;	// size in "file image of the segment"
	uint memsz;		// size in "memory image of the segment"
	uint flags;
	uint align;
} Elf32_Phdr;

_Static_assert(sizeof(Elf32_Phdr) == sizeof(uint) * 8,
	"Elf32_Phdr can't have gaps");

#endif
