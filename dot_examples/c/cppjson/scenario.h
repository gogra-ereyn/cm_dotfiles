#ifndef SCENARIO_H
#define SCENARIO_H

#include <stdint.h>

#define SCENARIO_MAX_LABELS 256u
#define SCENARIO_MAX_ALERTS 64u
#define SCENARIO_MAX_FILE_SIZE (64u * 1024u)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SC_APP_STATUS_UNKNOWN = 0,
	SC_APP_STATUS_UP = 1,
	SC_APP_STATUS_WEAK = 2,
	SC_APP_STATUS_DEAD = 3,
	SC_APP_STATUS_ENDED = 4
} app_status_t;

typedef enum {
	SC_ALERT_LEVEL_UNKNOWN = 0,
	SC_ALERT_LEVEL_POSITIVE = 1,
	SC_ALERT_LEVEL_INFORMATION = 2,
	SC_ALERT_LEVEL_WARNING = 3,
	SC_ALERT_LEVEL_CRITICAL = 4,
	SC_ALERT_LEVEL_FATAL = 5
} alert_level_t;

typedef struct {
	const char *value;
} scenario_label_t;

typedef struct {
	uint64_t handle;
	const char *name;
	const char *host_name;
	app_status_t status;
	uint32_t label_start;
	uint32_t label_count;
} app_spec_t;

typedef struct {
	const char *title;
	const char *body;
	alert_level_t level;
	uint32_t label_start;
	uint32_t label_count;
	uint32_t count;
	uint32_t period_ms;
	uint32_t has_at_ms;
	uint64_t at_ms;
} alert_spec_t;

typedef struct {
	app_spec_t app;
	alert_level_t default_level;
	uint32_t has_default_level;
	uint32_t default_label_start;
	uint32_t default_label_count;
	alert_spec_t alerts[SCENARIO_MAX_ALERTS];
	uint32_t alert_count;
	scenario_label_t labels[SCENARIO_MAX_LABELS];
	uint32_t label_count;
} scenario_t;

#ifdef __cplusplus
}
#endif

#endif
