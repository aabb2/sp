#ifndef FDS
#define FDS
	struct fds_pair {
		int in_use;
		int client_fd;
		int remote_fd;
		int n_times;
		unsigned long ip;
		unsigned int port;
	};
	int fds_init();
	static struct fds_pair *find_empty();
	int fds_add_client(int fd);
	int fds_set_times(int client_fd, int n);
	int fds_remove(int fd);
	int fds_find_by_fd(int fd, struct fds_pair *pair, int *is_client);
	int fds_set_remote(int client_fd, int remote_fd, unsigned long ip, unsigned int port);
#endif
