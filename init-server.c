#include <netinet/in.h>
#include <sys/socket.h>
#include "sp.h"

#define PORT_NUM 50000

int init_server() {
	int fd;
	struct sockaddr_in address;
	int result;
	size_t server_len;
	fd = socket(AF_INET, SOCK_STREAM, 0);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT_NUM);
	server_len = sizeof(address);

	result = bind(fd, (struct sockaddr *)&address, server_len);
	if(-1 == result) {
		return -1;
	}
	if(-1 == listen(fd, 5)) {
		return -1;
	}
	return fd;
}
