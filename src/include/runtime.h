/* runtime.c is the common runtime code shared by
   mips.twoints, mips.array, and any other
   mips emulators as needed */


#include <stdint.h>

/* All values taken from mips.twoints/mips.array */
#define MIPS_MERLCOOKIE 0x10000002
#define MIPS_RETURNADDR 0x8123456c
#define MIPS_MEMSIZE    0x01000000
#define MIPS_EXITCODE   0x000000ff /* exit code used to report an error */
#define MIPS_STDIN      0xffff0004
#define MIPS_STDOUT     0xffff000c

/* Note: This is in practice not changeable without an extensive rewrite */
#define MIPS_NUMREGS 32


/* Immediate Instructions */
#define MIPS_LW    0x23
#define MIPS_SW    0x2b
#define MIPS_BEQ   0x04
#define MIPS_BNE   0x05

/* Register Instructions */
#define MIPS_ADD   0x20
#define MIPS_SUB   0x22
#define MIPS_MULT  0x18
#define MIPS_MULTU 0x19
#define MIPS_DIV   0x1a
#define MIPS_DIVU  0x1b
#define MIPS_MFHI  0x10
#define MIPS_MFLO  0x12
#define MIPS_LIS   0x14
#define MIPS_SLT   0x2a
#define MIPS_SLTU  0x2b
#define MIPS_JR    0x08
#define MIPS_JALR  0x09

/* The runtime environment struct
   which stores useful information
   that people ought to know about */
struct rte {
  uint32_t *memory; /* The giant memory array */
  uint32_t offset;  /* The first byte in memory which contains code */
  uint32_t codeend; /* The end of the code segment */
  uint32_t registers[MIPS_NUMREGS]; /* The various MIPS registers.
                                       All will be initialized to zero except $30 and $31. */
  uint32_t pc; /* The program counter */
  uint32_t hi;
  uint32_t lo;

  uint8_t safemode; /* Adds additional error conditions if turned on */
};

/* mips_init should be called to set up the address space and such.
   Once it does its magic, it returns the runtime environment to the user,
   who can do further configuration and then should call mips_run once
   they're done whatever they need to do (such as loading information into registers). */
void mips_run(struct rte runtime);

struct rte mips_init(const char *fname,uint32_t offset);
