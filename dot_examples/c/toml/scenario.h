#ifndef SCENARIO_H
#define SCENARIO_H

#include <stdint.h>

#define SCENARIO_MAX_LABELS 256u
#define SCENARIO_MAX_ALERTS 64u
#define SCENARIO_MAX_LABELS_PER_ALERT 64u
#define SCENARIO_MAX_FILE_SIZE (1u << 16)

typedef enum {
	APP_STATUS_UNKNOWN = 0,
	APP_STATUS_UP = 1,
	APP_STATUS_WEAK = 2,
	APP_STATUS_DEAD = 3,
	APP_STATUS_ENDED = 4
} app_status_t;

typedef enum {
	ALERT_LEVEL_UNKNOWN = 0,
	ALERT_LEVEL_POSITIVE = 1,
	ALERT_LEVEL_INFORMATION = 2,
	ALERT_LEVEL_WARNING = 3,
	ALERT_LEVEL_CRITICAL = 4,
	ALERT_LEVEL_FATAL = 5
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

int scenario_parse_buffer(char *buf, uint32_t len, scenario_t *out);

int scenario_load_from_path(const char *path, scenario_t *out, char *buf, uint32_t buf_cap,
			    uint32_t *out_len);
#endif
