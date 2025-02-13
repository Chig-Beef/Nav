#include "Optimiser.h"
#include "Node.h"
#include "Panic.h"
#include "String.h"
#include "list.h"
#include <stdio.h>

void optimiserInit(Optimiser *o, Node funs) { o->src = funs; }

// NOTE: All optimisation methods return whether they produced a change, if so,
// then further optimisation could take place from earlier steps, and therefore
// the optimiser should go back to the start and optimise again

typedef struct Variable {
  String *name;
  bool used;
  Node *decBlock;
  int decIndex;
} Variable;

NEW_LIST_TYPE(Variable, Var)

#define SET_VAR_CHANGED                                                        \
  for (int k = 0; k < vars->len; ++k) {                                        \
    if (strEql(varName, vars->p[k].name)) {                                    \
      vars->p[k].used = true;                                                  \
      break;                                                                   \
    }                                                                          \
  }

void scrubVariable(Optimiser *o, String *name, Node *n, Node *parent,
                   int index) {
  Node *assignment;
  Node *ident;
  String *other;

  switch (n->kind) {
  case N_VAR_DEC:
    assignment = n->children.p;

    // Don't even look at new assignment
    if (assignment->kind == N_ASSIGNMENT) {
      if (assignment->children.p[0].kind == N_CREMENT) {
        assignment = assignment->children.p;
        ident = assignment->children.p + 1;
        if (ident->kind == N_ACCESS) {
          ident = ident->children.p;
        }
        other = ident->data;

        if (strEql(name, other)) {
          if (NodeListRemoveAt(&parent->children, index)) {
            panic("Couldn't remove from nodelist\n");
          }
        }
      } else {
        ident = assignment->children.p;
        if (ident->kind == N_ACCESS) {
          ident = ident->children.p;
        }
        other = ident->data;

        if (strEql(name, other)) {
          if (NodeListRemoveAt(&parent->children, index)) {
            panic("Couldn't remove from nodelist\n");
          }
        }
      }
    }
    break;

  case N_FOR_LOOP:
    if (n->children.p[2].kind == N_ASSIGNMENT) {
      scrubVariable(o, name, n->children.p + 2, n, 2);
    }

    if (n->children.p[n->children.len - 3].kind == N_ASSIGNMENT) {
      scrubVariable(o, name, n->children.p + n->children.len - 3, n,
                    n->children.len - 3);
    }

    scrubVariable(o, name, n->children.p + n->children.len - 1, n,
                  n->children.len - 1);
    break;

  case N_IF_BLOCK:
    scrubVariable(o, name, n->children.p + 4, n, 4);

    int i = 5;

    while (i < n->children.len) {
      if (n->children.p[i].kind == N_ELIF) {
        scrubVariable(o, name, n->children.p + i + 4, n, i + 4);
      } else {
        scrubVariable(o, name, n->children.p + i + 1, n, i + 1);
        break;
      }

      i += 5;
    }
    break;

  case N_SWITCH_STATE:
    for (int i = 5; i < n->children.len - 1; ++i) {
      scrubVariable(o, name, n->children.p + i, n, i);
    }

    break;

  case N_CASE_BLOCK:
    for (int i = 3; i < n->children.len; ++i) {
      scrubVariable(o, name, n->children.p + i, n, i);
    }
    break;

  case N_DEFAULT_BLOCK:
    for (int i = 2; i < n->children.len; ++i) {
      scrubVariable(o, name, n->children.p + i, n, i);
    }
    break;

  case N_BLOCK:
    for (int i = 1; i < n->children.len - 1; ++i) {
      scrubVariable(o, name, n->children.p + i, n, i);
    }
    break;

  default:
    // Do nothing
    break;
  }
}

void removeVariable(Optimiser *o, Variable v) {
  printf("Removing variable %s\n", v.name->data);
  scrubVariable(o, v.name, v.decBlock, NULL, 0);

  // Remove the original declaration
  if (NodeListRemoveAt(&v.decBlock->children, v.decIndex)) {
    panic("Couldn't remove from nodelist\n");
  }
}

bool variableEliminationExpression(Optimiser *o, Node *expr, VarList *vars) {
  // printf("Eliminating variables (expression) %s\n",
  // nodeCodeString(expr->kind));
  bool changed = false;

  return changed;
}

bool variableEliminationBlock(Optimiser *o, Node *block, VarList *vars);

bool variableEliminationStatement(Optimiser *o, Node *state, Node *block, int i,
                                  VarList *vars) {
  // printf("Eliminating variables (statement) %s\n",
  // nodeCodeString(state->kind));
  bool changed = false;

  Node *assignment, *fn, *index, *expr;
  Variable v;
  int j;

  switch (state->kind) {
  case N_VAR_DEC:
    assignment = state->children.p;
    if (assignment->kind == N_NEW_ASSIGNMENT) {
      // Check expression
      changed |= variableEliminationExpression(
          o, assignment->children.p + assignment->children.len - 1, vars);

      // Add new variable
      v = (Variable){assignment->children.p[2].data, false, block, i};
      if (VarListAppend(vars, v)) {
        panic("Couldn't append to varlist\n");
      }

    } else if (assignment->children.p[0].kind != N_CREMENT) {
      // First check the index if there is one
      if (assignment->children.p[1].kind == N_INDEX) {
        index = assignment->children.p + 1;
        changed |=
            variableEliminationExpression(o, index->children.p + 1, vars);
      }

      // Then check the expression
      changed |= variableEliminationExpression(
          o, assignment->children.p + assignment->children.len - 1, vars);
    } else { // Crement
             // Do nothing, doesn't count as using the variable
    }
    break;

  case N_LONE_CALL:
    // Check each arg
    fn = state->children.p;
    for (j = 3; j < fn->children.len; j += 2) {
      changed |= variableEliminationExpression(o, fn->children.p + j, vars);
    }
    break;

  case N_FOR_LOOP:
    // Check first assignment, condition, second assignment, and block
    j = 2;
    assignment = state->children.p + j;

    if (assignment->kind == N_NEW_ASSIGNMENT) {
      // Check expression
      changed |= variableEliminationExpression(
          o, assignment->children.p + assignment->children.len - 1, vars);

      // Add new variable
      v = (Variable){assignment->children.p[2].data, false, state, j};
      if (VarListAppend(vars, v)) {
        panic("Couldn't append to varlist\n");
      }
      j += 2;
    } else if (assignment->kind == N_ASSIGNMENT) {
      if (assignment->children.p[0].kind != N_CREMENT) {
        // First check the index if there is one
        if (assignment->children.p[1].kind == N_INDEX) {
          index = assignment->children.p + 1;
          changed |=
              variableEliminationExpression(o, index->children.p + 1, vars);
        }

        // Then check the expression
        changed |= variableEliminationExpression(
            o, assignment->children.p + assignment->children.len - 1, vars);
      } else { // Crement
               // Do nothing, doesn't count as using the variable
      }
      j += 2;
    } else {
      ++j;
    }

    expr = state->children.p + j;
    if (expr->kind == N_EXPRESSION) {
      changed |= variableEliminationExpression(o, expr, vars);
      j += 2;
    } else {
      ++j;
    }

    assignment = state->children.p + j;
    if (assignment->kind == N_ASSIGNMENT) {
      if (assignment->children.p[0].kind != N_CREMENT) {
        // First check the index if there is one
        if (assignment->children.p[1].kind == N_INDEX) {
          index = assignment->children.p + 1;
          changed |=
              variableEliminationExpression(o, index->children.p + 1, vars);
        }

        // Then check the expression
        changed |= variableEliminationExpression(
            o, assignment->children.p + assignment->children.len - 1, vars);
      } else { // Crement
               // Do nothing, doesn't count as using the variable
      }
    }

    changed |= variableEliminationBlock(
        o, state->children.p + state->children.len - 1, vars);

    break;

  case N_IF_BLOCK:
    // Check condition and block, and repeat for each other case
    changed |= variableEliminationExpression(o, state->children.p + 2, vars);

    changed |= variableEliminationBlock(o, state->children.p + 4, vars);

    j = 5;

    while (j < state->children.len) {
      if (state->children.p[j].kind == N_ELIF) {
        changed |=
            variableEliminationExpression(o, state->children.p + j + 2, vars);

        changed |= variableEliminationBlock(o, state->children.p + j + 4, vars);

        j += 5;
      } else { // Else
        changed |= variableEliminationBlock(o, state->children.p + j + 1, vars);
        break;
      }
    }
    break;

  case N_RET_STATE:
    if (assignment->children.p[1].kind == N_EXPRESSION) {
      changed |= variableEliminationExpression(o, state->children.p + 1, vars);
    }
    break;

  case N_SWITCH_STATE:
    // Check item, and all cases + default.
    changed |= variableEliminationExpression(o, state->children.p + 2, vars);

    j = 5;

    // NOTE: Calling 'variableEliminationBlock' with caseBlock and default block
    // is fine

    Node *caseBlock = state->children.p + j;
    while (caseBlock->kind == N_CASE_BLOCK) {
      changed |=
          variableEliminationExpression(o, caseBlock->children.p + 1, vars);

      int varLen = vars->len;

      // For every statement
      for (j = 3; j < caseBlock->children.len; ++j) {
        Node *state = caseBlock->children.p + j;
        changed |= variableEliminationStatement(o, state, caseBlock, j, vars);
      }

      // Cleanup vars
      while (vars->len > varLen) {
        j = varLen - 1;
        v = vars->p[j];

        // Remove from code
        if (!v.used) {
          removeVariable(o, v);
        }

        // Remove from vars
        if (VarListRemoveAt(vars, vars->len - 1)) {
          panic("Couldn't remove from list\n");
        }
      }

      ++j;
      caseBlock = state->children.p + j;
    }

    if (caseBlock->kind == N_DEFAULT_BLOCK) {
      int varLen = vars->len;

      // For every statement
      for (j = 2; j < caseBlock->children.len; ++j) {
        Node *state = caseBlock->children.p + j;
        changed |= variableEliminationStatement(o, state, caseBlock, j, vars);
      }

      // Cleanup vars
      while (vars->len > varLen) {
        j = varLen - 1;
        v = vars->p[j];

        // Remove from code
        if (!v.used) {
          removeVariable(o, v);
        }

        // Remove from vars
        if (VarListRemoveAt(vars, vars->len - 1)) {
          panic("Couldn't remove from list\n");
        }
      }
    }

    break;

  case N_BREAK_STATE:
    // Do nothing
    break;

  case N_CONTINUE_STATE:
    // Do nothing
    break;

  default:
    panic("Invalid statement in block");
  }

  return changed;
}

bool variableEliminationBlock(Optimiser *o, Node *block, VarList *vars) {
  // printf("Eliminating variables (block) %s\n", nodeCodeString(block->kind));

  bool changed = false;
  int varLen = vars->len;

  // For every statement
  for (int i = 1; i < block->children.len - 1; ++i) {
    Node *state = block->children.p + i;
    changed |= variableEliminationStatement(o, state, block, i, vars);
  }

  // Cleanup vars
  while (vars->len > varLen) {

    int i = vars->len - 1;
    Variable v = vars->p[i];

    // Remove from code
    if (!v.used) {
      removeVariable(o, v);
    }

    // Remove from vars
    if (VarListRemoveAt(vars, vars->len - 1)) {
      panic("Couldn't remove from list\n");
    }
  }

  return changed;
}

bool variableElimination(Optimiser *o) {
  printf("Eliminating variables\n");
  bool changed = false;

  VarList vars;
  if (VarListInit(&vars, 1)) {
    panic("Couldn't init varlist\n");
  }

  // For every function
  for (int i = 0; i < o->src.children.len; ++i) {
    Node *fn = o->src.children.p + i;

    // Optimise the block
    changed |= variableEliminationBlock(
        o, fn->children.p + fn->children.len - 1, &vars);
  }

  return changed;
}

bool branchElimination(Optimiser *o) { return false; }

bool constantFolding(Optimiser *o) { return false; }

bool strengthReduction(Optimiser *o) { return false; }

void optimise(Optimiser *o) {
RETRY_OPTIMISATION:

  if (variableElimination(o)) {
    goto RETRY_OPTIMISATION;
  }

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
