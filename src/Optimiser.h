#pragma once

#include "Node.h"

typedef struct Optimiser {
  Node src;
} Optimiser;

void optimiserInit(Optimiser *o, Node funs);

void optimise(Optimiser *o);
