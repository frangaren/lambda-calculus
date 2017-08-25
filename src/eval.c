#include "eval.h"

#include <stddef.h>
#include <stdio.h>

static bool _apply_in_place (Expression *function, Expression *argument,
  int depth);
static bool _rebase(Expression argument, int rebase_depth, int depth);

bool step_in_place (Expression *expression) {
  switch ((*expression)->type) {
    case VARIABLE:
      return false;
      break;
    case LAMBDA:
      return step_in_place(&(*expression)->lambda);
      break;
    case APPLICATION: {
      Expression *function = &(*expression)->application->function;
      Expression *argument = &(*expression)->application->argument;
      if ((*function)->type == LAMBDA) {
        Expression reduced_function =  (*function)->lambda;
        _rebase(reduced_function, -1, 1);
        (*function)->lambda = NULL;
        _apply_in_place(&reduced_function, argument, 0);
        free_expression(expression);
        *expression = reduced_function;
        return true;
      } else if (step_in_place(function)) {
        return true;
      } else if (step_in_place(argument)) {
        return true;
      } else {
        return false;
      }
      break;
    }
    default:
      return false;
  }
}

static bool _apply_in_place (Expression *function,
  Expression *argument, int depth)
{
  switch ((*function)->type) {
    case VARIABLE:
      if ((*function)->variable == depth) {
        free_expression(function);
        *function = copy_expression(*argument);
        _rebase(*function, depth, 0);
        return true;
      } else {
        return false;
      }
      break;
    case LAMBDA:
      return _apply_in_place(&(*function)->lambda, argument, depth + 1);
      break;
    case APPLICATION: {
      Expression *left = &(*function)->application->function;
      Expression *right = &(*function)->application->argument;
      bool left_outcome = _apply_in_place(left, argument,  depth);
      bool right_outcome = _apply_in_place(right, argument,  depth);
      return left_outcome || right_outcome;
      break;
    }
    default:
      return false;
  }
}

static bool _rebase(Expression expression, int rebase_depth, int depth) {
  if (rebase_depth == 0) return false;
  switch (expression->type) {
    case VARIABLE:
      if (expression->variable >= depth) {
        expression->variable += rebase_depth;
        return true;
      } else {
        return false;
      }
      break;
    case LAMBDA:
      return _rebase(expression->lambda, rebase_depth, depth + 1);
      break;
    case APPLICATION: {
      bool left = _rebase(expression->application->function, rebase_depth,
        depth);
      bool right = _rebase(expression->application->argument, rebase_depth,
        depth);
      return left || right;
      break;
    }
    default:
      return false;
      break;
  }
}

void step_eval_in_place (Expression *expression) {
  print_expression(*expression);
  bool change = false;
  do {
    change = step_in_place(expression);
    if (change) {
      printf(" ⟶\n\t");
      print_expression(*expression);
    }
  } while (change);
  printf("\n");
}