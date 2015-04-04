/*
 * mips_array accepts an array length (placed in register 2)
 * and a number of arguments (placed in register 1).
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "runtime.h"

int main(int argc, const char **argv) {
  uint8_t safety = 0;
  uint32_t offset = 0;
  uint32_t i;
  const char *fname = NULL;
  struct rte runtime;
  for (i=1;i<argc;i++) {
    if (!strcmp(argv[i],"-o")) {
      i++;
      if (i==argc || !sscanf(argv[i],"%x",&offset)) {
        fprintf(stderr,"-o should be followed by an offset specified in hex.\n");
        exit(MIPS_EXITCODE);
      }
    } else if (!strcmp(argv[i],"-s")) {
      safety = 1;
    } else if (!fname) {
      fname = argv[i];
    } else {
      fname = NULL;
      break;
    }
  }
  if (!fname) {
    fprintf(stderr,"Usage: mips-array <filename> [-o offset]\n");
    exit(MIPS_EXITCODE);
  }
  runtime = mips_init(fname,offset);
  fprintf(stderr,"Enter number of elements: ");
  if (!scanf(" %d",&runtime.registers[2])) {
    fprintf(stderr,"Expected an integer argument for number of elements.\n");
    goto error;
  }
  offset = runtime.codeend;
  for (i=0;i<runtime.registers[2];i++) {
    if (offset+i >= MIPS_MEMSIZE) {
      fprintf(stderr,"No memory for further array elements!\n");
      goto error;
    }
    fprintf(stderr,"Enter array element %d: ",i);
    if (!scanf(" %d",&runtime.memory[offset+i])) {
      fprintf(stderr,"Expected an integer argument for register 2.\n");
      goto error;
    }
  }
  offset *= 4;
  runtime.registers[1]  = offset;
  runtime.registers[30] = MIPS_MEMSIZE;
  runtime.safemode = safety;
  mips_run(runtime);
  return 0;

error:
  free(runtime.memory);
  exit(MIPS_EXITCODE);
}
