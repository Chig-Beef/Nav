#include "Optimiser.h"

void initOptimiser(Optimiser *o, Node funs) { o->src = funs; }

// NOTE: All optimisation methods return whether they produced a change, if so,
// then further optimisation could take place from earlier steps, and therefore
// the optimiser should go back to the start and optimise again

bool variableElimination(Optimiser *o) { return false; }

bool branchElimination(Optimiser *o) { return false; }

bool constantFolding(Optimiser *o) { return false; }

bool strengthReduction(Optimiser *o) { return false; }

void optimise(Optimiser *o) {
RETRY_OPTIMISATION:

  // First step doesn't need to try again if a change is produced
  variableElimination(o);

  if (branchElimination(o)) {
    goto RETRY_OPTIMISATION;
  }

  if (constantFolding(o)) {
    goto RETRY_OPTIMISATION;
  }

  if (strengthReduction(o)) {
    goto RETRY_OPTIMISATION;
  }
}
