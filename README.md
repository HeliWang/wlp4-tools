# wlp4-tools
A collection of silly wlp4 tools based on University of Waterloo's Foundations of Sequential Programs (CS241) second-year CS course.

These are a collection of silly wlp4 tools inspired by various jokes amongst my friends and various members of the [Programming Languages Group](http://plg.uwaterloo.ca) at the University of Waterloo, started while I was TAing [CS241](http://www.student.cs.uwaterloo.ca/~cs241/).

WLP4 is CS241's toy C-like language, which is based upon a toy MIPS32-based assembly language which is about as minimalistic as possible while still being Turing-complete. This repository is intended for various tools which compile WLP4 code, assemble the resulting assembly, run a virtual machine or otherwise, with an emphasis on esoteric languages or solutions to problems.

Also included are the non-silly runtime environments mips-array, mips-twoints, and mips-io, which are experimental improvements upon the existing java course tools, supporting toggleable safety features and improved reading from standard input. mips-twoints and mips-array work exactly how mips.twoints and mips.array do. mips-io expects to have a wlp4 program which accepts int*,int and passes 0xffff0004,0 as the arguments: 0xffff0004 is stdin, and 0 is just an unused value. This is the only way as far as I can tell to actually get a pointer to standard input without trying to do something extremely hacky like figure out the correct stack pointer offset in wain. They're written in C because it seemed like a good idea at the time.

Pages of interest:

[The WLP4 Language Specification](https://www.student.cs.uwaterloo.ca/~cs241/wlp4/WLP4.html)  
[The WLP4 SLR(1) Machine](https://www.student.cs.uwaterloo.ca/~cs241/wlp4/WLP4.lr1)  
[The course Assembly Language](http://www.student.cs.uwaterloo.ca/~cs241/mips/mipsref.pdf)
