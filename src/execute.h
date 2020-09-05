#ifndef EXECUTE_H
#define EXECUTE_H

#include "parse.h"
#include "value.h"
#include "map.h"

value_t *execute_expression(expression_t *expressions, map_t *variables);

#endif
