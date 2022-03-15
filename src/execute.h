#ifndef EXECUTE_H
#define EXECUTE_H

#include "value.h"
#include "map.h"
#include "string.h"

map_t *empty_overloads(void);
map_t *empty_variables(void);
value_t *execute_script(string_t *document, value_t *arguments, map_t *variables, map_t *overloads);

#endif
