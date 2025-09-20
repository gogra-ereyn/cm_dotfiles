#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int setup_listening_socket_v4(int port)
{
	struct sockaddr_in srv_addr = {};
	int fd, enable, ret;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		perror("socket()");
		return -1;
	}

	enable = 1;
	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	if (ret < 0) {
		perror("setsockopt(SO_REUSEADDR)");
		close(fd);
		return -1;
	}

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(port);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(fd, (const struct sockaddr *)&srv_addr, sizeof(srv_addr));
	if (ret < 0) {
		perror("bind()");
		close(fd);
		return -1;
	}

	if (listen(fd, 1024) < 0) {
		perror("listen()");
		close(fd);
		return -1;
	}

	return fd;
}

int setup_listening_socket_choose(int port, int ipv6)
{
	struct sockaddr_in srv_addr = {};
	struct sockaddr_in6 srv_addr6 = {};
	int fd, enable, ret, domain;

	if (ipv6)
		domain = AF_INET6;
	else
		domain = AF_INET;

	fd = socket(domain, SOCK_STREAM, 0);
	if (fd == -1) {
		perror("socket()");
		return -1;
	}

	enable = 1;
	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	if (ret < 0) {
		perror("setsockopt(SO_REUSEADDR)");
		return -1;
	}

	if (ipv6) {
		srv_addr6.sin6_family = AF_INET6;
		srv_addr6.sin6_port = htons(port);
		srv_addr6.sin6_addr = in6addr_any;
		ret = bind(fd, (const struct sockaddr *)&srv_addr6,
			   sizeof(srv_addr6));
	} else {
		srv_addr.sin_family = AF_INET;
		srv_addr.sin_port = htons(port);
		srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		ret = bind(fd, (const struct sockaddr *)&srv_addr,
			   sizeof(srv_addr));
	}

	if (ret < 0) {
		perror("bind()");
		return -1;
	}

	if (listen(fd, 1024) < 0) {
		perror("listen()");
		return -1;
	}

	return fd;
}

int setup_listening_socket_retport(int port, uint16_t *out_port)
{
	struct sockaddr_in addr = { 0 };
	socklen_t len = sizeof(addr);
	int fd, enable, ret;

	fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	enable = 1;
	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	if (ret < 0) {
		close(fd);
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		close(fd);
		return -1;
	}

	if (listen(fd, 1024) < 0) {
		close(fd);
		return -1;
	}

	if (getsockname(fd, (struct sockaddr *)&addr, &len) < 0) {
		close(fd);
		return -1;
	}
	if (len < sizeof(addr)) {
		close(fd);
		errno = EINVAL;
		return -1;
	}

	*out_port = ntohs(addr.sin_port);
	return fd;
}

int setup_listening_socket_randport(uint16_t *port)
{
	return setup_listening_socket_retport(0, port);
}

int bind_free_port(int fd, struct sockaddr_in *addr)
{
	socklen_t addrlen;
	int ret;

	addr->sin_port = 0;
	if (bind(fd, (struct sockaddr *)addr, sizeof(*addr)))
		return -errno;

	addrlen = sizeof(*addr);
	ret = getsockname(fd, (struct sockaddr *)addr, &addrlen);
	if (ret)
		return 1;

	if (!addr->sin_port)
		return 1;

	return 0;
}

int main(int argc, char **argv)
{
	int fd;
	fd = setup_listening_socket_choose(9224, 0);
	if (fd <= 0) {
		fprintf(stderr, "bidsad no mad\n");
		return 1;
	}
}
