#include <netdb.h>
#include "sp.h"

int get_ip(char *host, struct in_addr *result) {
	struct hostent *hostinfo;
	char **address;
	hostinfo = gethostbyname(host);
	if(!hostinfo) {
		return -1;
	}
	address = hostinfo->h_addr_list;
	while(*address) {
		result->s_addr = (*(struct in_addr *)*address).s_addr;
		return 0;
	}
	return -1;
}

