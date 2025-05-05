#pragma once

#include "Node.h"
#include "StringManager.h"

typedef struct Optimiser {
  Node src;
  StringManager *sm;
} Optimiser;

void optimiserInit(Optimiser *o, Node funs, StringManager *sm);

void optimise(Optimiser *o);
