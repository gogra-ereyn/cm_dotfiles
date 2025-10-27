#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>

#define MAX_ARGS 256
#define MAX_PATH_LEN 4096
#define MAX_CMD_LEN 8192
#define MAX_MOUNTS 32

typedef enum {
	PROJECT_C,
	PROJECT_CPP,
	PROJECT_CMAKE,
	PROJECT_AUTOTOOLS,
	PROJECT_UNKNOWN
} project_type_t;

typedef struct {
	project_type_t type;
	const char *image;
	const char *description;
} image_map_t;

typedef struct {
	char source[MAX_PATH_LEN];
	char target[MAX_PATH_LEN];
	const char *options;
} mount_spec_t;

typedef struct {
	project_type_t project_type;
	const char *container_name;
	const char *image;
	char cwd[MAX_PATH_LEN];
	char project_root[MAX_PATH_LEN];
	uid_t uid;
	gid_t gid;
	int keep_container;
	int verbose;
	mount_spec_t mounts[MAX_MOUNTS];
	int num_mounts;
} config_t;

static const image_map_t image_lookup[] = {
	{ PROJECT_C, "localhost/c-dev:latest", "C projects (Makefile)" },
	{ PROJECT_CPP, "localhost/cpp-dev:latest", "C++ projects" },
	{ PROJECT_CMAKE, "localhost/cmake-dev:latest", "CMake projects" },
	{ PROJECT_AUTOTOOLS, "localhost/auto-dev:latest", "Autotools projects" },
	{ PROJECT_UNKNOWN, "localhost/dev:latest", "Generic development" }
};

static const size_t image_lookup_size = sizeof(image_lookup) / sizeof(image_lookup[0]);

static project_type_t detect_project_type(const char *path)
{
	struct stat st;
	char filepath[MAX_PATH_LEN];

	snprintf(filepath, sizeof(filepath), "%s/CMakeLists.txt", path);
	if (stat(filepath, &st) == 0) {
		return PROJECT_CMAKE;
	}

	snprintf(filepath, sizeof(filepath), "%s/configure.ac", path);
	if (stat(filepath, &st) == 0) {
		return PROJECT_AUTOTOOLS;
	}
	snprintf(filepath, sizeof(filepath), "%s/configure.in", path);
	if (stat(filepath, &st) == 0) {
		return PROJECT_AUTOTOOLS;
	}

	snprintf(filepath, sizeof(filepath), "%s/Makefile", path);
	if (stat(filepath, &st) == 0) {
		FILE *f;
		char line[256];
		int has_cxx = 0;

		f = fopen(filepath, "r");
		if (f) {
			while (fgets(line, sizeof(line), f)) {
				if (strstr(line, "CXX") || strstr(line, "g++") ||
				    strstr(line, "c++") || strstr(line, ".cpp")) {
					has_cxx = 1;
					break;
				}
			}
			fclose(f);
		}

		return has_cxx ? PROJECT_CPP : PROJECT_C;
	}

	/* Scan for source files as fallback */
	DIR *dir;
	struct dirent *entry;
	int has_cpp = 0, has_c = 0;

	dir = opendir(path);
	if (dir) {
		while ((entry = readdir(dir))) {
			const char *name;
			size_t len;

			name = entry->d_name;
			len = strlen(name);

			if (len > 4 && strcmp(name + len - 4, ".cpp") == 0) {
				has_cpp = 1;
			} else if (len > 3 && strcmp(name + len - 3, ".cc") == 0) {
				has_cpp = 1;
			} else if (len > 4 && strcmp(name + len - 4, ".cxx") == 0) {
				has_cpp = 1;
			} else if (len > 2 && strcmp(name + len - 2, ".c") == 0) {
				has_c = 1;
			}
		}
		closedir(dir);
	}

	if (has_cpp)
		return PROJECT_CPP;
	if (has_c)
		return PROJECT_C;

	return PROJECT_UNKNOWN;
}

static int find_project_root(const char *start_path, char *root_path, size_t root_size)
{
	char current[MAX_PATH_LEN];
	char parent[MAX_PATH_LEN];
	struct stat st;
	const char *markers[] = { ".git", "Makefile", "CMakeLists.txt", "configure.ac" };
	size_t i;

	if (realpath(start_path, current) == NULL) {
		return -1;
	}

	while (1) {
		/* Check for project markers */
		for (i = 0; i < sizeof(markers) / sizeof(markers[0]); i++) {
			char marker_path[MAX_PATH_LEN];
			snprintf(marker_path, sizeof(marker_path), "%s/%s", current, markers[i]);
			if (stat(marker_path, &st) == 0) {
				snprintf(root_path, root_size, "%s", current);
				return 0;
			}
		}

		/* move to parent directory */
		snprintf(parent, sizeof(parent), "%s/..", current);
		if (realpath(parent, parent) == NULL) {
			break;
		}

		/* stop at filesystem root */
		if (strcmp(current, parent) == 0) {
			break;
		}

		strcpy(current, parent);
	}

	/* No project root found, use start path */
	snprintf(root_path, root_size, "%s", start_path);
	return 0;
}

/*
 * img name for project type
 */
static const char *get_image_for_type(project_type_t type)
{
	size_t i;

	for (i = 0; i < image_lookup_size; i++) {
		if (image_lookup[i].type == type) {
			return image_lookup[i].image;
		}
	}

	return image_lookup[image_lookup_size - 1].image; /* fallback to UNKNOWN */
}

/*
 * Add a mount specification
 */
static int add_mount(config_t *config, const char *source, const char *target, const char *options)
{
	mount_spec_t *mount;

	if (config->num_mounts >= MAX_MOUNTS) {
		fprintf(stderr, "Error: Maximum number of mounts (%d) exceeded\n", MAX_MOUNTS);
		return -1;
	}

	mount = &config->mounts[config->num_mounts];
	snprintf(mount->source, sizeof(mount->source), "%s", source);
	snprintf(mount->target, sizeof(mount->target), "%s", target);
	mount->options = options;

	config->num_mounts++;
	return 0;
}

/*
 * default mounts for the configuration
 */
static int setup_mounts(config_t *config)
{
	char ccache_dir[MAX_PATH_LEN];
	char home_cache[MAX_PATH_LEN];
	struct stat st;

	/* mount proj root */
	if (add_mount(config, config->project_root, config->project_root, "Z") != 0) {
		return -1;
	}

	/* mount ccache directory if it exists.
	 * TODO - tf is the state of distc again?
	 * */
	snprintf(ccache_dir, sizeof(ccache_dir), "%s/.ccache", getenv("HOME"));
	if (stat(ccache_dir, &st) == 0 && S_ISDIR(st.st_mode)) {
		if (add_mount(config, ccache_dir, "/ccache", "Z") != 0) {
			return -1;
		}
	}

	/* mount .cache  */
	snprintf(home_cache, sizeof(home_cache), "%s/.cache", getenv("HOME"));
	if (stat(home_cache, &st) == 0 && S_ISDIR(st.st_mode)) {
		char cache_target[MAX_PATH_LEN];
		snprintf(cache_target, sizeof(cache_target), "/home/user/.cache");
		if (add_mount(config, home_cache, cache_target, "Z") != 0) {
			return -1;
		}
	}

	return 0;
}

/*
 * check if a container is running
 */
static int is_container_running(const char *name)
{
	char cmd[512];
	int status;

	snprintf(cmd, sizeof(cmd),
		 "podman ps --filter name=^%s$ --format '{{.Names}}' 2>/dev/null | grep -q '^%s$'",
		 name, name);

	status = system(cmd);
	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/*
 * Start a persistent container
 */
static int start_container(const config_t *config)
{
	char cmd[MAX_CMD_LEN];
	char *p;
	size_t remaining;
	int i;
	int status;

	p = cmd;
	remaining = sizeof(cmd);

	/* base command */
	p += snprintf(p, remaining, "podman run -d --name %s ", config->container_name);
	remaining = sizeof(cmd) - (p - cmd);

	/* uid/gid mapping - bare important bruv */
	p += snprintf(p, remaining, "--userns=keep-id ");
	remaining = sizeof(cmd) - (p - cmd);

	for (i = 0; i < config->num_mounts; i++) {
		const mount_spec_t *mount;
		mount = &config->mounts[i];
		if (mount->options) {
			p += snprintf(p, remaining, "-v %s:%s:%s ", mount->source, mount->target,
				      mount->options);
		} else {
			p += snprintf(p, remaining, "-v %s:%s ", mount->source, mount->target);
		}
		remaining = sizeof(cmd) - (p - cmd);
	}

	p += snprintf(p, remaining, "-w %s ", config->cwd);
	remaining = sizeof(cmd) - (p - cmd);

	/* env vars */
	p += snprintf(p, remaining, "-e CCACHE_DIR=/ccache ");
	remaining = sizeof(cmd) - (p - cmd);

	/* img + keepalive cmd */
	p += snprintf(p, remaining, "%s sleep infinity", config->image);

	if (config->verbose) {
		fprintf(stderr, "Starting container: %s\n", cmd);
	}

	status = system(cmd);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		fprintf(stderr, "Error: Failed to start container\n");
		return -1;
	}

	return 0;
}

/*
 * Execute command in container
 */
static int exec_in_container(const config_t *config, int argc, char **argv)
{
	char *exec_argv[MAX_ARGS];
	int exec_argc;
	int i;
	pid_t pid;
	int status;

	exec_argc = 0;

	/* Build podman exec command */
	exec_argv[exec_argc++] = "podman";
	exec_argv[exec_argc++] = "exec";
	exec_argv[exec_argc++] = "-it";
	exec_argv[exec_argc++] = "-w";
	exec_argv[exec_argc++] = config->cwd;
	exec_argv[exec_argc++] = (char *)config->container_name;

	for (i = 0; i < argc && exec_argc < MAX_ARGS - 1; i++) {
		exec_argv[exec_argc++] = argv[i];
	}
	exec_argv[exec_argc] = NULL;

	if (config->verbose) {
		fprintf(stderr, "Executing in container:");
		for (i = 0; i < exec_argc; i++) {
			fprintf(stderr, " %s", exec_argv[i]);
		}
		fprintf(stderr, "\n");
	}

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		execvp("podman", exec_argv);
		perror("execvp");
		exit(127);
	}

	if (waitpid(pid, &status, 0) < 0) {
		perror("waitpid");
		return -1;
	}

	if (WIFEXITED(status)) {
		return WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
		return 128 + WTERMSIG(status);
	}

	return -1;
}

/*
 * eggsecute command in ephemeral container
 */
static int run_ephemeral(const config_t *config, int argc, char **argv)
{
	char cmd[MAX_CMD_LEN];
	char *p;
	size_t remaining;
	int i;
	int status;

	p = cmd;
	remaining = sizeof(cmd);

	p += snprintf(p, remaining, "podman run --rm -it ");
	remaining = sizeof(cmd) - (p - cmd);

	/* mapping */
	p += snprintf(p, remaining, "--userns=keep-id ");
	remaining = sizeof(cmd) - (p - cmd);

	for (i = 0; i < config->num_mounts; i++) {
		const mount_spec_t *mount;
		mount = &config->mounts[i];
		if (mount->options) {
			p += snprintf(p, remaining, "-v %s:%s:%s ", mount->source, mount->target,
				      mount->options);
		} else {
			p += snprintf(p, remaining, "-v %s:%s ", mount->source, mount->target);
		}
		remaining = sizeof(cmd) - (p - cmd);
	}

	p += snprintf(p, remaining, "-w %s ", config->cwd);
	remaining = sizeof(cmd) - (p - cmd);

	p += snprintf(p, remaining, "-e CCACHE_DIR=/ccache ");
	remaining = sizeof(cmd) - (p - cmd);

	p += snprintf(p, remaining, "%s ", config->image);
	remaining = sizeof(cmd) - (p - cmd);

	for (i = 0; i < argc && remaining > 2; i++) {
		size_t arg_len;
		arg_len = strlen(argv[i]);

		if (strchr(argv[i], ' ') || strchr(argv[i], '"') || strchr(argv[i], '\'')) {
			p += snprintf(p, remaining, "'%s' ", argv[i]);
		} else {
			p += snprintf(p, remaining, "%s ", argv[i]);
		}
		remaining = sizeof(cmd) - (p - cmd);
	}

	if (config->verbose) {
		fprintf(stderr, "Running ephemeral: %s\n", cmd);
	}

	status = system(cmd);
	if (WIFEXITED(status)) {
		return WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
		return 128 + WTERMSIG(status);
	}

	return -1;
}

/*
 * Print usage information
 */
static void print_usage(const char *prog)
{
	size_t i;

	fprintf(stderr, "Usage: %s [OPTIONS] COMMAND [ARGS...]\n\n", prog);
	fprintf(stderr, "Execute commands in a project-appropriate container.\n\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -e, --ephemeral     Use ephemeral container (default: persistent)\n");
	fprintf(stderr, "  -i, --image IMAGE   Override container image\n");
	fprintf(stderr, "  -n, --name NAME     Container name (default: dev-main)\n");
	fprintf(stderr, "  -v, --verbose       Verbose output\n");
	fprintf(stderr, "  -h, --help          Show this help\n\n");
	fprintf(stderr, "Detected project types and images:\n");

	for (i = 0; i < image_lookup_size; i++) {
		fprintf(stderr, "  %-20s -> %s\n", image_lookup[i].description,
			image_lookup[i].image);
	}

	fprintf(stderr, "\nUID/GID Mapping:\n");
	fprintf(stderr, "  Uses --userns=keep-id to map your host UID/GID into the container.\n");
	fprintf(stderr,
		"  This ensures files created in mounted volumes have correct ownership.\n");
}

int main(int argc, char **argv)
{
	config_t config;
	int i;
	int cmd_start;
	int exit_code;

	memset(&config, 0, sizeof(config));
	config.container_name = "dev-main";
	config.image = NULL;
	config.keep_container = 1;
	config.verbose = 0;
	config.uid = getuid();
	config.gid = getgid();

	if (getcwd(config.cwd, sizeof(config.cwd)) == NULL) {
		perror("getcwd");
		return 1;
	}

	if (find_project_root(config.cwd, config.project_root, sizeof(config.project_root)) != 0) {
		fprintf(stderr, "Error: Could not determine project root\n");
		return 1;
	}

	config.project_type = detect_project_type(config.project_root);

	cmd_start = 1;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_usage(argv[0]);
			return 0;
		} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
			config.verbose = 1;
			cmd_start++;
		} else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--ephemeral") == 0) {
			config.keep_container = 0;
			cmd_start++;
		} else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--image") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "Error: --image requires an argument\n");
				return 1;
			}
			config.image = argv[++i];
			cmd_start += 2;
		} else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--name") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "Error: --name requires an argument\n");
				return 1;
			}
			config.container_name = argv[++i];
			cmd_start += 2;
		} else {
			break;
		}
	}

	if (cmd_start >= argc) {
		fprintf(stderr, "Error: No command specified\n\n");
		print_usage(argv[0]);
		return 1;
	}

	if (config.image == NULL) {
		config.image = get_image_for_type(config.project_type);
	}

	if (setup_mounts(&config) != 0) {
		return 1;
	}

	if (config.verbose) {
		const char *type_names[] = { "C", "C++", "CMake", "Autotools", "Unknown" };
		fprintf(stderr, "Configuration:\n");
		fprintf(stderr, "  Project type: %s\n", type_names[config.project_type]);
		fprintf(stderr, "  Project root: %s\n", config.project_root);
		fprintf(stderr, "  Working dir:  %s\n", config.cwd);
		fprintf(stderr, "  Image:        %s\n", config.image);
		fprintf(stderr, "  Container:    %s\n", config.container_name);
		fprintf(stderr, "  UID:GID:      %u:%u\n", config.uid, config.gid);
		fprintf(stderr, "  Persistent:   %s\n", config.keep_container ? "yes" : "no");
		fprintf(stderr, "  Mounts:       %d\n", config.num_mounts);
		for (i = 0; i < config.num_mounts; i++) {
			fprintf(stderr, "    %s -> %s (%s)\n", config.mounts[i].source,
				config.mounts[i].target,
				config.mounts[i].options ? config.mounts[i].options : "default");
		}
		fprintf(stderr, "\n");
	}

	if (config.keep_container) {
		if (!is_container_running(config.container_name)) {
			if (config.verbose) {
				fprintf(stderr, "Container not running, starting...\n");
			}
			if (start_container(&config) != 0) {
				return 1;
			}
		}
		exit_code = exec_in_container(&config, argc - cmd_start, argv + cmd_start);
	} else {
		exit_code = run_ephemeral(&config, argc - cmd_start, argv + cmd_start);
	}

	return exit_code;
}
