#ifndef SCENARIO_JSON_H
#define SCENARIO_JSON_H

#include <stddef.h>
#include "scenario.h"

#ifdef __cplusplus
extern "C" {
#endif

int scenario_load_from_json(const char *json, size_t len, scenario_t *out, char *string_buf,
			    size_t string_buf_cap, size_t *string_buf_used);

#ifdef __cplusplus
}
#endif

#endif
