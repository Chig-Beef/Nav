#pragma once

#include "Node.h"
#include "Parser.h"

typedef struct Hoister {
  Node enums, structs, funcs;
} Hoister;

void hoist(Hoister *h, Parser parsers[], int parserCount);
