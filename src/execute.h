#ifndef EXECUTE_H
#define EXECUTE_H

#include "value.h"
#include "map.h"

value_t *execute_script(char *document, value_t *arguments, map_t *globals);

#endif
