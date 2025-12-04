#include <string.h>
#include <stdio.h>

extern "C" {
#include "scenario.h"
}
#include "scenario_json.h"

#define TEST_ASSERT(cond)                                                                       \
	do {                                                                                    \
		if (!(cond)) {                                                                  \
			fprintf(stderr, "TEST FAILED: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
			return 1;                                                               \
		}                                                                               \
	} while (0)

static int test_basic_json(void)
{
	const char *json = R"JSON(
{
  "app": {
    "handle": 42,
    "name": "demo-app",
    "host_name": "demo-host",
    "status": "Up",
    "labels": ["env:test","service:demo"]
  },
  "alert_defaults": {
    "level": "Warning",
    "labels": ["env:test","service:demo"]
  },
  "alerts": [
    {
      "title": "disk 80% used",
      "body": "Disk almost full on /var",
      "level": "Warning",
      "labels": ["component:disk"],
      "count": 3,
      "period_ms": 1000
    },
    {
      "title": "CPU overload",
      "body": "5m load average > 16",
      "level": "Critical",
      "labels": ["component:cpu","priority:high"],
      "at_ms": 10000
    }
  ]
}
)JSON";

	scenario_t sc;
	char arena[64 * 1024];
	size_t used;
	int rc;

	used = 0;
	rc = scenario_load_from_json(json, strlen(json), &sc, arena, sizeof arena, &used);
	TEST_ASSERT(rc == 0);

	TEST_ASSERT(sc.app.handle == 42u);
	TEST_ASSERT(strcmp(sc.app.name, "demo-app") == 0);
	TEST_ASSERT(strcmp(sc.app.host_name, "demo-host") == 0);
	TEST_ASSERT(sc.app.label_count == 2u);
	TEST_ASSERT(strcmp(sc.labels[0].value, "env:test") == 0);
	TEST_ASSERT(strcmp(sc.labels[1].value, "service:demo") == 0);

	TEST_ASSERT(sc.has_default_level == 1u);
	TEST_ASSERT(sc.default_level == ALERT_LEVEL_WARNING);
	TEST_ASSERT(sc.alert_count == 2u);

	TEST_ASSERT(strcmp(sc.alerts[0].title, "disk 80% used") == 0);
	TEST_ASSERT(sc.alerts[0].count == 3u);
	TEST_ASSERT(sc.alerts[0].period_ms == 1000u);

	TEST_ASSERT(strcmp(sc.alerts[1].title, "CPU overload") == 0);
	TEST_ASSERT(sc.alerts[1].has_at_ms == 1u);
	TEST_ASSERT(sc.alerts[1].at_ms == 10000u);

	return 0;
}

int main(void)
{
	int rc;
	rc = test_basic_json();
	if (rc != 0)
		return rc;
	printf("JSON scenario test passed\n");
	return 0;
}
