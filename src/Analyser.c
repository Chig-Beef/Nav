#include "Ident.h"
#include "Node.h"

typedef struct Analyser {
  // Source
  Node enums, structs, funcs;

  // Defined variables, types, etc
  Stack vars;
} Analyser;

void analyserInit(Analyser *a, Node enums, Node structs, Node funcs) {
  a->enums = enums;
  a->structs = structs;
  a->funcs = funcs;
  a->vars = (Stack){NULL, 0};
}

void analyseEnums(Analyser *a) {}

void analyseStructs(Analyser *a) {}

void analyseFuncs(Analyser *a) {}

void analyse(Analyser *a) {
  analyseEnums(a);
  analyseStructs(a);
  analyseFuncs(a);
}
