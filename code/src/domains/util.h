#ifndef UTIL_H
#define UTIL_H

#include "../limit.h"
#include "../parser/parser.h"
#include "../dataflow/cfg.h"
#include "../hardware_specification.h"

#define EVALUATION_STACK_SIZE 1000

#define PR_ARITH(p) (p + (1-p)/(MAXINT-MININT+1))
#define PR_BOOL(p) (p + (1-p)/2)

#endif