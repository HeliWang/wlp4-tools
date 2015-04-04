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

static void mips_printregs(struct rte *runtime) {
  int i;
  int j;
  /* This is a bit hacky, but since we know that pc comes right after the register array
     in runtime we can just read one past the end of the array to get the pc as a register. */
  for (i=1;i<=MIPS_NUMREGS;) {
    for (j=0;j<4&&i<=MIPS_NUMREGS;j++,i++) {
      char name[4];
      name[2] = '\0';
      if (i==MIPS_NUMREGS) {
        memset(name,' ',3);
        name[0] = 'p';
        name[1] = 'c';
      } else {
        sprintf(name,"$%02d",i);
      }
      /* Identical format to mips.twoints/mips.array */
      fprintf(stderr,"%s = %08x   ",name,runtime->registers[i]);
    }
    fprintf(stderr,"\n");
  }
  free(runtime->memory);
}

static void mips_error(const char *msg, struct rte *runtime) {
  fprintf(stderr,"%s",msg);
  mips_printregs(runtime);
  exit(MIPS_EXITCODE);
}

static uint32_t check_div4(uint32_t in,struct rte *runtime) {
  if (in%4 != 0) {
    mips_error("Runtime Error: Tried to access memory at a location not divisible by 4.\n",runtime);
  }
  return in/4;
}

static void mips_assert(int assertion, struct rte *runtime) {
  if (!assertion) {
    mips_error("Runtime Error: Tried to execute unrecognized instruction.\n",runtime);
  }
}

/* Starts running the mips emulator. Asks for the end of the code so that it
   can automatically throw an error if the user tries to write to the code
   segment or their code falls off the end of the code segment. */
void mips_run(struct rte runtime) {
  uint32_t instr;
  uint8_t s, t, d; /* Registers */
  int16_t imm; /* immediate value */
  uint32_t work;
  runtime.pc = runtime.offset;
  /* The Fetch-Execute Cycle (TM) */
  uint32_t lastpc = 0;
  while (1) {
    runtime.registers[0] = 0; /* Ensure that $0 is always 0 */
    if (runtime.pc >= runtime.codeend && runtime.safemode) {
    if (runtime.pc == MIPS_RETURNADDR) {
      break;
    }
      mips_error("Runtime Error: PC got past end of program code.\n",&runtime);
    }
    /* See MIPS reference sheet for a full description of each command */
    instr = runtime.memory[check_div4(runtime.pc,&runtime)];
    lastpc = runtime.pc;
    runtime.pc += 4;
    uint8_t opcode = instr >> 26;
    s = (instr >> 21) & 0x1f;
    t = (instr >> 16) & 0x1f;
    if (opcode) {
      imm = instr & 0xffff;
      switch (opcode) {
      case MIPS_LW:
      case MIPS_SW:
        work = runtime.registers[s]+imm;
        if (work < runtime.codeend && runtime.safemode) {
          mips_error("Runtime Error: Tried to access memory in the code segment.\n",&runtime);
        }
        if (work >= MIPS_MEMSIZE) {
          if (opcode == MIPS_LW && work == MIPS_STDIN) {
            runtime.registers[t] = 0;
            int out = getchar();
            if (out == EOF) {
              runtime.registers[t] = -1;
            } else {
              runtime.registers[t] = out;
            }
          } else if (opcode == MIPS_SW && work == MIPS_STDOUT) {
            printf("%c",(char)(runtime.registers[t] & 0xff));
          } else {
            mips_error("Runtime Error: Tried to access memory past end of memory.\n",&runtime);
          }
        } else {
          work = check_div4(work,&runtime);
          if (opcode == MIPS_SW) {
            runtime.memory[work] = runtime.registers[t];
          } else {
            runtime.registers[t] = runtime.memory[work];
          }
        }
        break;
      case MIPS_BEQ:
        if (runtime.registers[s] == runtime.registers[t]) {
          runtime.pc += 4 * imm;
        }
        break;
      case MIPS_BNE:
        if (runtime.registers[s] != runtime.registers[t]) {
          runtime.pc += 4 * imm;
        }
        break;
      default:
        mips_assert(0,&runtime);
        break;
      }
    } else {
      opcode = instr & 0x3f;
      d = (instr >> 11) & 0x1f;
      mips_assert((instr & 0x7c0) == 0,&runtime); /* Check to see unused bits are zero */
      switch (opcode) {
      case MIPS_ADD:
        runtime.registers[d] = runtime.registers[s] + runtime.registers[t];
        break;
      case MIPS_SUB:
        runtime.registers[d] = runtime.registers[s] - runtime.registers[t];
        break;
      case MIPS_SLT:
        runtime.registers[d] = (int32_t)runtime.registers[s] < (int32_t)runtime.registers[t] ? 1 : 0;
        break;
      case MIPS_SLTU:
        runtime.registers[d] = runtime.registers[s] < runtime.registers[t] ? 1 : 0;
        break;
      case MIPS_MULT:
        mips_assert(d == 0,&runtime);
        {
          uint64_t multr = (uint64_t)((int64_t)(int32_t)runtime.registers[s] * (int64_t)(int32_t)runtime.registers[t]);
          runtime.hi = (uint32_t)(multr >> 32);
          runtime.lo = (uint32_t)(multr & 0xFFFFFFFF);
        }
        break;
      case MIPS_MULTU:
        mips_assert(d == 0,&runtime);
        {
          uint64_t multr = ((uint64_t)runtime.registers[s] * (uint64_t)runtime.registers[t]);
          runtime.hi = (uint32_t)(multr >> 32);
          runtime.lo = (uint32_t)(multr & 0xFFFFFFFF);
        }
        break;
      case MIPS_DIV:
        mips_assert(d == 0,&runtime);
        runtime.hi = (int32_t)runtime.registers[s] % (int32_t)runtime.registers[t];
        runtime.lo = (int32_t)runtime.registers[s] / (int32_t)runtime.registers[t];
        break;
      case MIPS_DIVU:
        mips_assert(d == 0,&runtime);
        runtime.hi = runtime.registers[s] % runtime.registers[t];
        runtime.lo = runtime.registers[s] / runtime.registers[t];
        break;
      case MIPS_JR:
        mips_assert(t == 0 && d == 0,&runtime);
        runtime.pc = runtime.registers[s];
        break;
      case MIPS_JALR:
        mips_assert(t == 0 && d == 0,&runtime);
        {
          int tmp = runtime.registers[s];
          runtime.registers[31] = runtime.pc;
          runtime.pc = tmp;
        }
        break;
      case MIPS_LIS:
        mips_assert(s == 0 && t == 0,&runtime);
        runtime.registers[d] = runtime.memory[runtime.pc/4];
        runtime.pc += 4;
        break;
      case MIPS_MFHI:
      case MIPS_MFLO:
        mips_assert(s == 0 && t == 0,&runtime);
        runtime.registers[d] = opcode == MIPS_MFHI ? runtime.hi : runtime.lo;
        break;
      default:
        mips_assert(0,&runtime);
        break;
      }
    }
  }
  fprintf(stderr,"MIPS execution completed normally.\n");
  mips_printregs(&runtime);
}
