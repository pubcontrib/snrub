#ifndef EXECUTE_H
#define EXECUTE_H

#include "value.h"
#include "map.h"
#include "string.h"

value_t *execute_script(string_t *document, value_t *arguments, map_t *globals);

#endif
