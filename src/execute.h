#ifndef EXECUTE_H
#define EXECUTE_H

#include "value.h"
#include "map.h"

value_t *execute_script(char *document, map_t *globals);

#endif
