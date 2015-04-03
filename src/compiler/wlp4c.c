//Must be run with mips-io.
//standard input should contain the WLP4 machine
//and then the program

//I/O Functions
int print(int *stdout, int *msg) {
  while (*msg != 0) {
    *stdout = *msg;
    msg = msg + 1;
  }
  return 0;
}
int printline(int *stdout, int *msg) {
  int t = 0;
  t = print(stdout,msg);
  *stdout = 10;
  return 0;
}

//Crashes the program by dereferencing NULL
int error() {
  int *nil = NULL;
  *nil = 5;
  return *nil;
}

//Reads an int from standard input
//Assumes the int is the only thing on the line
//Intended to be used with the .lr file format
int readInt(int *stdin) {
  int resl = 0;
  int nextC = 0;
  nextC = *stdin;
  while (nextC != 10) {
    if (nextC == 0) {
      resl = error();
    } else {}
    nextC = nextC - 48;
    if (nextC < 0) {
      resl = error();
    } else {}
    if (nextC > 9) {
      resl = error();
    } else {}
    resl = resl*10 + nextC;
  }
  return resl;
}

//WLP4 Dynamic Arrays
//The dynamic array starts 2 words BEFORE the pointer!
//Dynamic array memory layout:
//size
//capacity
//<elements>

int dynarrSize(int *dynarr) {
  return *(dynarr-2);
}
int dynarrDelete(int *dynarr) {
  delete [] (dynarr - 2);
  return 0;
}
int dynarrNew(int *ref) {
  int *dynarr = NULL;
  dynarr = new int[2];
  *dynarr = 0;
  *(dynarr+1) = 0;
  dynarr = dynarr + 2;
  return dynarr - ref;
}
//Makes a dynamic version of an array. Intended for strings
//as an efficiency/convenience improvement.
int dynarrNewString(int *arr,int len) {
  int *dynarr = NULL;
  int *dynarrP = NULL;
  dynarr = new int[len + 3];
  *dynarr = len+1;
  *(dynarr+1) = len+1;
  dynarr = dynarr + 2;
  dynarrP = dynarr;
  while (len > 0) {
    *dynarr = *arr;
    dynarr = dynarr + 1;
    arr = arr + 1;
    len = len - 1;
  }
  *dynarr = 0;
  return dynarrP - arr;
}
int dynarrPush(int *dynarr, int elem) {
  int capacity = 0;
  int size = 0;
  int *outdynarr = NULL;
  int i = 0;
  capacity = *(dynarr-1);
  size = *(dynarr-2);
  
  if (capacity == size) {
    if (capacity == 0) {
      capacity = 5;
      outdynarr = new int[capacity + 2];
      outdynarr = outdynarr + 2;
    } else {
      capacity = capacity * 2;
      outdynarr = new int[capacity + 2];
      outdynarr = outdynarr + 2;
      i = 0;
      while (i < size) {
        *(outdynarr+i) = *(dynarr+i);
        i = i + 1;
      }
    }
    delete [] (dynarr - 2);
  } else {
    outdynarr = dynarr;
  }
  *(outdynarr + size) = elem;
  size = size + 1;
  *(outdynarr - 2) = size;
  *(outdynarr - 1) = capacity;
  return outdynarr - dynarr;
}

//WLP4 Trees
//Arrays of size 2
//Element 1: Pointer to node contents dynamic array
//Element 2: Pointer to children dynamic array
int treeNew(int *ref,int *node) {
  int *tree = NULL;
  tree = new int[2];
  *tree = node - tree;
  *(tree+1) = dynarrNew(tree);
  return tree - ref;
}
int treeDelete(int *tree) {
  int children = 0;
  int numChildren = 0;
  int *dynarr = NULL;
  dynarr = *(tree+1)+tree;
  numChildren = *(dynarr-2);
  while (children < numChildren) {
    treeDelete(*(dynarr+children)+tree);
    children = children + 1;
  }
  numChildren = dynarrDelete(*tree + tree);
  numChildren = dynarrDelete(dynarr);
  delete [] tree;
  return 0;
}
int treeAddChild(int *tree,int *child) {
  int *dynarr = NULL;
  dynarr = *(tree+1) + tree;
  dynarr = dynarrPush(dynarr,child - tree);
  *(tree+1) = dynarr - tree;
  return 0;
}
int treeGetChild(int *tree,int index) {
  int *dynarr = NULL;
  dynarr = *(tree+1) + tree;
  return *(dynarr+index);
}
int treeGetNode(int *tree) {
  return *tree;
}

//IO part 2
//Uses dynamic arrays as strings.

//Reads a line from standard input
//Returns it stored in a dynamic array
int readLine(int *stdin) {
  int dynarr = NULL;
  int chr = 0;
  dynarr = dynarrNew(stdin)+stdin;
  chr = *stdin;
  while (chr != 10) {
    if (chr == 0) {
      chr = error();
    } else {}
    dynarr = dynarrPush(dynarr,chr) + dynarr;
    chr = *stdin;
  }
  return dynarr - stdin;
}

//Garbage skipper in lr file
int skipGarbage(int *stdin) {
  int lines = 0;
  int *t = NULL;
  int te = 0;
  lines = readInt(stdin);
  while (lines > 0) {
    t = readLine(stdin) + stdin;
    te = dynarrDelete(t);
    lines = lines - 1;
  }
  return 0;
}

//Reads the LR1 machine into a hashtable
int readMachine(int *stdin) {
  int *machine = NULL;
  int moves = 0;
  int *line = NULL;
  int *linea = NULL;
  int t = 0;
  moves = readInt(stdin);
  machine = new int[moves+1];
  *machine = moves;
  while (moves > 0) {
    line = readLine(stdin);
    linea = new int[4];
    //Unimplemented: TODO
    //*linea = strReadInt(line);
    t = dynarrDelete(line);
    moves = moves - 1;
  }
  return machine - stdin;
}

int tokensPush(int *stdin,int *tokens,int *curType,int *curStr) {
  int *ntokens = NULL;
  int *tarr = NULL;
  if (curType != NULL) {
    if (curStr == NULL) {
      *ntokens = error();
    } else {}
    tarr = new int[2];
    *tarr = curType - tarr;
    *(tarr+1) = curStr - tarr;
    ntokens = dynarrPush(tokens,tarr-stdin) + tokens;
  } else {}
  return ntokens - tokens;
}

//Scanner
int lex(int *stdin) {
  int chr = 0;
  int *tokens = NULL;
  int *curStr = NULL;
  int *curType = NULL;
  int *tarr = NULL;
  int matchingState = 0;
  tokens = dynarrNew(stdin) + stdin;
  chr = *stdin;
  while (chr != 0) {
    //state 0: match anything
    //and figure out what to do with it
    if (matchingState == 0) {
      //Space
      if (chr == 32) {
        tokens = tokensPush(stdin,tokens,curStr,curType) + tokens;
      } else {}
      //!
      if (chr == 33) {
        curType = NULL + 1;
        curStr = NULL;
      } else {}
      //%
      if (chr == 37) {
        tarr = new int[3];
        *tarr = 80;
        *(tarr+1) = 67;
        *(tarr+2) = 84;
        curType = dynarrNewString(tarr,3) + tarr;
        *tarr = 37;
        curStr = dynarrNewString(tarr,1) + tarr;
        delete [] tarr;
        tokens = tokensPush(stdin,tokens,curStr,curType) + tokens;
      } else {}
      //&
      if (chr == 38) {
        tarr = new int[3];
        *tarr = 65;
        *(tarr+1) = 77;
        *(tarr+2) = 80;
        curType = dynarrNewString(tarr,3) + tarr;
        *tarr = 38;
        curStr = dynarrNewString(tarr,1) + tarr;
        delete [] tarr;
        tokens = tokensPush(stdin,tokens,curStr,curType) + tokens;
      } else {}
      //(
      if (chr == 40) {
        tarr = new int[6];
        *tarr = 76;
        *(tarr+1) = 80;
        *(tarr+2) = 65;
        *(tarr+3) = 82;
        *(tarr+4) = 69;
        *(tarr+5) = 78;
        curType = dynarrNewString(tarr,6) + tarr;
        *tarr = 40;
        curStr = dynarrNewString(tarr,1) + tarr;
        delete [] tarr;
        tokens = tokensPush(stdin,tokens,curStr,curType) + tokens;
      } else {}
      //)
      if (chr == 41) {
        tarr = new int[6];
        *tarr = 82;
        *(tarr+1) = 80;
        *(tarr+2) = 65;
        *(tarr+3) = 82;
        *(tarr+4) = 69;
        *(tarr+5) = 78;
        curType = dynarrNewString(tarr,6) + tarr;
        *tarr = 41;
        curStr = dynarrNewString(tarr,1) + tarr;
        delete [] tarr;
        tokens = tokensPush(stdin,tokens,curStr,curType) + tokens;
      } else {}
    } else {}
    if (matchingState == 1) {
    } else {}
    matchingState = 0 - matchingState;
    chr = *stdin;
  }
  return tokens - stdin;
}

int wain(int *stdin, int unused) {
  int *stdout = NULL;
  int *dynarr = NULL;
  int *tokens = NULL;
  int *machine = NULL;
  stdout = stdin+2;
  // Body
  //Skip terminals
  unused = skipGarbage(stdin);
  //Skip nonterminals
  unused = skipGarbage(stdin);
  //Skip start nonterminal
  tokens = readLine(stdin) + stdin;
  unused = dynarrDelete(tokens);
  //Skip rules - we'll hardcode them
  unused = skipGarbage(stdin);
  //Skip number of states - Unneeded
  unused = readInt(stdin);
  //Read SLR1 machine
  machine = readMachine(stdin) + stdin;
  tokens = lex(stdin) + stdin;
  // End
  delete [] machine;
  return 0;
}
