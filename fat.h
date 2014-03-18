#ifndef _MACH_O_FAT_H_
#define _MACH_O_FAT_H_
#include <stdint.h>

typedef int         vm_prot_t;
typedef int         integer_t;
typedef integer_t   cpu_type_t;
typedef integer_t   cpu_subtype_t;
#define FAT_MAGIC	0xcafebabe
#define FAT_CIGAM	0xbebafeca

struct fat_header {
    uint32_t	magic;		/* FAT_MAGIC */
    uint32_t	nfat_arch;	/* number of structs that follow */
};

struct fat_arch {
    cpu_type_t	cputype;	/* cpu specifier (int) */
    cpu_subtype_t	cpusubtype;	/* machine specifier (int) */
    uint32_t	offset;		/* file offset to this object file */
    uint32_t	size;		/* size of this object file */
    uint32_t	align;		/* alignment as a power of 2 */
};

#endif /* _MACH_O_FAT_H_ */
