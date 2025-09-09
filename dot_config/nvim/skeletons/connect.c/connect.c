#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int tcp_connect(char *host, char *port)
{
	struct addrinfo hints, *res, *rp;
	int sockfd = -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	int status = getaddrinfo(host, port, &hints, &res);
	if (status != 0)
		return -1;

	for (rp = res; rp != NULL; rp = rp->ai_next) {
		sockfd =
			socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd == -1)
			continue;

		if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;

		close(sockfd);
		sockfd = -1;
	}

	freeaddrinfo(res);
	return sockfd;
}

int tcp_connect_u16(char *addr, uint16_t port) {
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%u", port);
    return tcp_connect(addr, port_str);
}
