/**
 * By Sean Harrap (sharrap), CS241 ISA, Winter 2015
 * Written for CS241 to replace java tools
 *
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "runtime.h"

/* Tries to divide an integer by 4. Throws an error if it fails. */
static uint32_t check_div4(uint32_t in,uint32_t *memory) {
  if (in%4 != 0) {
    fprintf(stderr,"Critical Error: Malformed table entry (not divisible by 4).\n");
    free(memory);
    exit(MIPS_EXITCODE);
  }
  return in/4;
}

static uint32_t swap_endian(uint32_t in) {
  return ((in >> 24) & 0xFF) | ((in >> 8) & 0xFF00) | ((in << 8) & 0xFF0000) | ((in << 24) & 0xFF000000);
}

static void mips_relocate(FILE *infile,uint32_t *memory,uint32_t offset,uint32_t tablelen) {
  uint32_t word;
  uint32_t i;
  uint32_t skip=0;
  uint32_t code=0;
  for (i=0;i<tablelen;i++) {
    if (!fread(&word,sizeof(uint32_t),1,infile)) {
      fprintf(stderr,"Critical Error: Unexpected end of MERL file before finishing relocation table segment. Perhaps your first line should not be .word 0x10000002 (beq $0, $0, 2)?\n");
      goto error;
    }
    word = swap_endian(word);
    /* Skip ESR/ESD letters */
    if (skip) {
      skip--;
      continue;
    }
    switch (code) {
    case 0x00: /* Not currently in an instruction */
      code = word;
      break;
    case 0x01: /* REL */
      word = check_div4(word,memory);
      memory[word+offset] = memory[word+offset] + offset*4 - 0xc; /* Subtract 12 since we didn't load the header into the actual code */
      code = 0;
      break;
    case 0x11: /* ESR */
      fprintf(stderr,"Warning: ESR entry found in MERL file. This file is likely not completely linked and may not behave correctly.\n");
    case 0x05: /* ESD */
      check_div4(word,memory);
      code = 0xDEADBEEF;
      break;
    case 0xDEADBEEF: /* Followup to ESR/ESD. Read length and skip that many bytes. */
      skip = word;
      code = 0;
      break;
    default:
      fprintf(stderr,"Critical Error: Unrecognized relocation table entry 0x%x.\n",code);
      goto error;
    }
  }

  return;

error:
  free(memory);
  exit(MIPS_EXITCODE);
}

static uint32_t mips_load_code(FILE *infile,uint32_t *memory,uint32_t offset) {
  uint32_t word = 0;
  uint32_t i;
  fread(&word,sizeof(uint32_t),1,infile);
  word = swap_endian(word);
  if (word == MIPS_MERLCOOKIE) {
    uint32_t lens[2];
    if (fread(lens,sizeof(uint32_t),2,infile) != 2) {
      fprintf(stderr,"Critical Error: Unexpected end of MERL file before finishing header. Perhaps your first line should not be .word 0x10000002 (beq $0, $0, 2)?\n");
      goto error;
    }
    lens[0]=swap_endian(lens[0]);
    lens[1]=swap_endian(lens[1]);
    /* This implementation doesn't actually load the MERL stuff into memory.
       This is purely a design decision. */
    uint32_t clen = (lens[1]-12)/4; /* Code length in words for the for loop */
    for (i=offset;i<clen;i++) {
      if (!fread(&word,sizeof(uint32_t),1,infile)) {
        fprintf(stderr,"Critical Error: Unexpected end of MERL file before finishing code segment. Perhaps your first line should not be .word 0x10000002 (beq $0, $0, 2)?\n");
        goto error;
      }
      memory[i] = swap_endian(word);
    }
    mips_relocate(infile,memory,offset,(lens[0]-lens[1]) >> 2);
  } else if (offset != 0) {
    fprintf(stderr,"Critical Error: Nonzero offset for non-relocatable code.\n");
    goto error;
  } else {
    /* Naively load the code into memory */
    memory[0] = word;
    for (i=1;fread(&word,sizeof(uint32_t),1,infile);i++) {
      memory[i] = swap_endian(word);
    }
  }
  return i; /* We only reach this point if i was initialized */

error:
  free(memory);
  exit(MIPS_EXITCODE);
}

/* Allocates a sufficiently large chunk of memory and fills it with zeroes
   Then loads the code from the specified file and relocates it by the provided
   offset.
   Returns the first non-code address in codeend and the memory as the return value. */
struct rte mips_init(const char *fname,uint32_t offset) {
  struct rte runtime;
  runtime.safemode = 0;
  memset(runtime.registers,0,MIPS_NUMREGS*sizeof(uint32_t));
  runtime.registers[30] = MIPS_MEMSIZE/sizeof(uint32_t);
  runtime.registers[31] = MIPS_RETURNADDR;
  runtime.pc = 0;
  runtime.hi = 0;
  runtime.lo = 0;
  if (offset%4 != 0) {
    fprintf(stderr,"Critical Error: Offset must be divisible by 4.\n");
    exit(MIPS_EXITCODE);
  }
  FILE *infile = fopen(fname,"r");
  if (!infile) {
    fprintf(stderr,"Critical Error: Mips emulator failed to open file %s.\n",fname);
    exit(MIPS_EXITCODE);
  }
  runtime.memory = calloc(MIPS_MEMSIZE/sizeof(uint32_t),sizeof(uint32_t));
  if (!runtime.memory) {
    fprintf(stderr,"Critical Error: Mips emulator failed to allocate sufficient memory.\n");
    fclose(infile);
    exit(MIPS_EXITCODE);
  }
  runtime.codeend = mips_load_code(infile,runtime.memory,offset/4) * 4;
  fclose(infile);
  runtime.offset = offset;
  return runtime;
}
