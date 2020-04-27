#include <string.h>
#include "fds.h"
#define FDS_MAX_LEN 1024


static struct fds_pair fds_set[FDS_MAX_LEN];

int fds_init() {
	memset((void *)fds_set, 0, sizeof(fds_set));
	return 0;
}

static struct fds_pair *find_empty() {
	int i;
	for(i = 0; i < FDS_MAX_LEN; i++) {
		if(!fds_set[i].in_use) {
			return fds_set + i;
		}
	}
	return (void *)0;
}

int fds_add_client(int fd) {
	struct fds_pair *p;
	if(!(p = find_empty())){
		return -1;
	}
	p->client_fd = fd;
	p->in_use = 1;
	return fd;
}

int fds_set_times(int client_fd, int n) {
	int i;
	for(i = 0; i < FDS_MAX_LEN; i++) {
		if(!fds_set[i].in_use) continue;
		if(fds_set[i].client_fd == client_fd) {
			fds_set[i].n_times = n;
			return 0;
		}
	}
	return -1;
}

int fds_remove(int fd) {
	int i;
	for(i = 0; i < FDS_MAX_LEN; i++) {
		if(!fds_set[i].in_use) continue;

		if((fds_set[i].client_fd == fd) || (fds_set[i].remote_fd == fd)) {
			memset((void *)(fds_set + i), 0, sizeof(struct fds_pair));
			return 0;
		}
	}
	return -1;
}

int fds_find_by_fd(int fd, struct fds_pair *pair, int *is_client) {
	int i;
	for(i = 0; i<FDS_MAX_LEN; i++) {
		if(!fds_set[i].in_use) continue;

		if((fd == fds_set[i].client_fd) || (fd == fds_set[i].remote_fd)) {
			memcpy((void *)pair, (void *)(fds_set + i), sizeof(struct fds_pair));
			*is_client = (fd == fds_set[i].client_fd) ? 1 : 0;
			return 0;
		}
	}
	return -1;
}

int fds_set_remote(int client_fd, int remote_fd, unsigned long ip, unsigned int port) {
	int i;
	for(i = 0; i<FDS_MAX_LEN; i++) {
		if(!fds_set[i].in_use) continue;

		if(fds_set[i].client_fd == client_fd) {
			fds_set[i].remote_fd = remote_fd;
			fds_set[i].ip = ip;
			fds_set[i].port = port;
			return 0;
		}
	}
	return -1;
}



















