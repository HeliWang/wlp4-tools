/*
 * mips twoints accepts two integers, which it stores
 * in registers 1 and 2.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "runtime.h"

int main(int argc, const char **argv) {
  uint8_t safety = 0;
  uint32_t offset = 0;
  const char *fname = NULL;
  uint32_t i;
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
    fprintf(stderr,"Usage: mips-io <filename> [-o offset]\n");
    exit(MIPS_EXITCODE);
  }
  runtime = mips_init(fname,offset);
  runtime.safemode = safety;
  runtime.registers[1] = 0xffff0004;
  runtime.registers[2] = 0;
  mips_run(runtime);
  return 0;
}
