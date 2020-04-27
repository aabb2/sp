#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include "sp.h"

int connect_to(struct in_addr *addr, unsigned int port) {
	int sockfd;
	int result, len;
	struct sockaddr_in address;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int flags;
	flags = fcntl(sockfd, F_GETFL);
	if (flags == -1) {
		 return -1;
	}
	flags |= O_NONBLOCK;
	if(-1 == fcntl(sockfd, F_SETFL, flags)) {
		return -1;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = addr->s_addr;
	address.sin_port = htons(port);
	len = sizeof(address);

	result = connect(sockfd, (struct sockaddr *)&address, len);

	if(result == -1 && errno != EINPROGRESS) {
		return -1;
	}

	flags &= ~O_NONBLOCK;
	if(-1 == fcntl(sockfd, F_SETFL, flags)) {
		return -1;
	}
	return sockfd;
}
