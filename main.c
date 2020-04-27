#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include "./log/log.h"
#include "sp.h"
#include "fds/fds.h"


int main() {
	if(log_init("/var/sp/") == -1) {
		perror("log init failed");
		exit(23);
	}
	printf("hello\n");
	int server_fd;
	if(-1 == (server_fd = init_server())) {
		perror("init server failed");
		exit(2);
	}
	printf("server fd: %d\n", server_fd);

	int epoll_fd;
	if(-1 == (epoll_fd = epoll_create(1))) {
		perror("epoll failed");
		exit(4);
	}
	printf("epoll create : %d\n", epoll_fd);

	char *hostname = "baidu.com";
	unsigned int port = 80;
	struct in_addr address;
	if(-1 == get_ip(hostname, &address)) {
		perror("get ip failed");
		return 6;
	}
	printf("get ip : %s\n", inet_ntoa(address));

	int remote_fd;
	remote_fd = connect_to(&address, port);
	if(remote_fd == -1) {
		perror("connect to failed");
		return 9;
	}
	printf("remote fd : %d\n", remote_fd);

	struct epoll_event ev;
	ev.data.fd = remote_fd;
	ev.events = EPOLLOUT | EPOLLRDHUP;
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, remote_fd, &ev) == -1){
		perror("epoll add failed");
		return 10;
	}
	int ready;
	struct epoll_event evlist[10];
	ready = epoll_wait(epoll_fd, evlist, 10, -1);
	if(ready == -1){
		perror("epoll wait failed");
		return 11;
	}
	int j;
	for(j = 0; j< ready; j++) {
		printf(" fd=%d; events: %s%s%s\n", evlist[j].data.fd,
		(evlist[j].events & EPOLLOUT) ? "EPOLLOUT " : "",
		(evlist[j].events & EPOLLHUP) ? "EPOLLHUP " : "",
		(evlist[j].events & EPOLLERR) ? "EPOLLERR " : "");

		if(evlist[j].events & EPOLLOUT) {
			printf(" write result : %d\n", write(evlist[j].data.fd, "a", 1));
		}

	}

	fds_init();
	int fds_result;

	fds_add_client(4);
	fds_set_times(4, 1);
	fds_set_remote(4, 5, ntohl(address.s_addr), port);
	fds_add_client(8);


	struct fds_pair temp_pair;
	int is_client;
	if(-1 == (fds_result = fds_find_by_fd(8, &temp_pair, &is_client))) {
		perror("find failed");
		return 23;
	}
	struct in_addr temp_addr;
	temp_addr.s_addr = htonl(temp_pair.ip);
	printf("fds_pair : %d %d %d %s %u\n", temp_pair.client_fd, temp_pair.remote_fd, temp_pair.n_times, inet_ntoa(temp_addr), temp_pair.port);

	close(server_fd);
	close(remote_fd);
	close(epoll_fd);
	return 0;
}
