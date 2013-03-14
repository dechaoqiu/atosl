#define CPU_TYPE_ANY        ((cpu_type_t) -1)

#define CPU_TYPE_VAX        ((cpu_type_t) 1)

/*
 *  * Capability bits used in the definition of cpu_type.
 *   */
#define CPU_ARCH_MASK   0xff000000      /*  mask for architecture bits */
#define CPU_ARCH_ABI64  0x01000000      /*  64 bit ABI * */
/*  skip             ((cpu_type_t) 2)    */
/*  skip             ((cpu_type_t) 3)    */
/*  skip             ((cpu_type_t) 4)    */
/*  skip             ((cpu_type_t) 5)    */
#define CPU_TYPE_MC680x0    ((cpu_type_t) 6)
#define CPU_TYPE_X86        ((cpu_type_t) 7)
#define CPU_TYPE_I386       CPU_TYPE_X86        /*  compatibility */
#define CPU_TYPE_X86_64     (CPU_TYPE_X86 | CPU_ARCH_ABI64)

/*  skip CPU_TYPE_MIPS       ((cpu_type_t) 8)    */
/*  skip             ((cpu_type_t) 9)    */
#define CPU_TYPE_MC98000    ((cpu_type_t) 10)
#define CPU_TYPE_HPPA           ((cpu_type_t) 11)
#define CPU_TYPE_ARM        ((cpu_type_t) 12)
#define CPU_TYPE_MC88000    ((cpu_type_t) 13)
#define CPU_TYPE_SPARC      ((cpu_type_t) 14)
#define CPU_TYPE_I860       ((cpu_type_t) 15)
/*  skip CPU_TYPE_ALPHA      ((cpu_type_t) 16)   */
/*  skip             ((cpu_type_t) 17)   */
#define CPU_TYPE_POWERPC        ((cpu_type_t) 18)
#define CPU_TYPE_POWERPC64      (CPU_TYPE_POWERPC | CPU_ARCH_ABI64)

/*
 *  ARM subtypes
 */
#define CPU_SUBTYPE_ARM_ALL             ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_ARM_V4T             ((cpu_subtype_t) 5)
#define CPU_SUBTYPE_ARM_V6              ((cpu_subtype_t) 6)
#define CPU_SUBTYPE_ARM_V5TEJ           ((cpu_subtype_t) 7)
#define CPU_SUBTYPE_ARM_XSCALE      ((cpu_subtype_t) 8)
#define CPU_SUBTYPE_ARM_V7      ((cpu_subtype_t) 9)
#define CPU_SUBTYPE_ARM_V7S      ((cpu_subtype_t) 11)


/*
 * PowerPC subtypes
 */
#define CPU_SUBTYPE_POWERPC_ALL     ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_POWERPC_601     ((cpu_subtype_t) 1)
#define CPU_SUBTYPE_POWERPC_602     ((cpu_subtype_t) 2)
#define CPU_SUBTYPE_POWERPC_603     ((cpu_subtype_t) 3)
#define CPU_SUBTYPE_POWERPC_603e    ((cpu_subtype_t) 4)
#define CPU_SUBTYPE_POWERPC_603ev   ((cpu_subtype_t) 5)
#define CPU_SUBTYPE_POWERPC_604     ((cpu_subtype_t) 6)
#define CPU_SUBTYPE_POWERPC_604e    ((cpu_subtype_t) 7)
#define CPU_SUBTYPE_POWERPC_620     ((cpu_subtype_t) 8)
#define CPU_SUBTYPE_POWERPC_750     ((cpu_subtype_t) 9)
#define CPU_SUBTYPE_POWERPC_7400    ((cpu_subtype_t) 10)
#define CPU_SUBTYPE_POWERPC_7450    ((cpu_subtype_t) 11)
#define CPU_SUBTYPE_POWERPC_970     ((cpu_subtype_t) 100)
