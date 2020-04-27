#ifndef SP
#define SP
#include <netinet/in.h>
	int init_server();
	int connect_to(struct in_addr *addr, unsigned int port);
	int get_ip(char *host, struct in_addr *result);
	int trans(int from_fd, int to_fd);
#endif
