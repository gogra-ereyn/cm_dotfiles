#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char **argv)
{
	const char *img = getenv("CTR_IMAGE");
	if (!img)
		img = "myproj-dev:latest";
	const char *name = getenv("CTR_NAME");
	if (!name)
		name = "myproj-dev";
	const char *work = getenv("CTR_WORKDIR");
	if (!work)
		work = "/w";
	const char *cache = getenv("CTR_CCACHE");
	if (!cache)
		cache = getenv("HOME");
	const char *net = getenv("CTR_NET");
	if (!net)
		net = "slirp4netns";
	const char *hostnet = getenv("CTR_HOSTNET");
	char pidbuf[32];
	snprintf(pidbuf, sizeof pidbuf, "%d", getpid());
	const char *home = getenv("HOME");
	const char *pwd = getenv("PWD");
	char cachebuf[4096];
	if (cache && strcmp(cache, getenv("HOME")) == 0) {
		size_t n =
			snprintf(cachebuf, sizeof cachebuf, "%s/.cache/ccache", home ? home : "");
		cache = n < sizeof cachebuf ? cachebuf : "/tmp/ccache";
	}
	int extra = hostnet && strcmp(hostnet, "1") == 0;
	int base = 20 + extra + argc;
	char **args = calloc(base + 1, sizeof(char *));
	int i = 0;
	args[i++] = "podman";
	args[i++] = "run";
	args[i++] = "--rm";
	args[i++] = "-it";
	args[i++] = "--name";
	char namebuf[256];
	snprintf(namebuf, sizeof namebuf, "%s-%s", name, pidbuf);
	args[i++] = namebuf;
	args[i++] = "--userns=keep-id";
	args[i++] = "--security-opt";
	args[i++] = "label=disable";
	args[i++] = "--env";
	args[i++] = "CCACHE_DIR=/ccache";
	char v1[8192], v2[8192];
	snprintf(v1, sizeof v1, "%s:%s:rshared,rw", pwd, work);
	args[i++] = "--volume";
	args[i++] = v1;
	snprintf(v2, sizeof v2, "%s:/ccache:rshared,rw", cache);
	args[i++] = "--volume";
	args[i++] = v2;
	args[i++] = "--workdir";
	args[i++] = work;
	args[i++] = "--network";
	args[i++] = net;
	if (extra) {
		args[i++] = "--network";
		args[i++] = "host";
	}
	args[i++] = (char *)img;
	for (int k = 1; k < argc; ++k)
		args[i++] = argv[k];
	args[i] = NULL;
	execvp("podman", args);
	return 127;
}
