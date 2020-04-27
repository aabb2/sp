#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>

#include "./log/log.h"
#include "sp.h"
#include "fds/fds.h"

#define MAX_EVENTS 10

void err_exit(char *msg) {
	log_err(msg);
	exit(2);
}

void clean(int fd, int epoll_fd);

int connect1(int client_fd);

int connect2(int client_fd, int *remote_fd);

int main() {
	if(log_init("/var/sp/") == -1) {
		err_exit("log init failed");
	}
	fds_init();
	int server_fd, result;
	if(-1 == (server_fd = init_server())) {
		err_exit("init server failed");
	}

	int epoll_fd;
	if(-1 == (epoll_fd = epoll_create1(0))) {
		err_exit("epoll");
	}
	struct epoll_event ev;
	ev.data.fd = server_fd;
	ev.events = EPOLLIN | EPOLLRDHUP;
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1){
		err_exit("epoll add server fd");
	}
	int ready, i;
	struct epoll_event evlist[MAX_EVENTS];
	struct sockaddr_in client_address;
	socklen_t client_len;
	int client_sockfd;
	char resp[512];
	while(1) {
		ready = epoll_wait(epoll_fd, evlist, MAX_EVENTS, -1);
		if(ready == -1) {
			if(errno == EINTR) {
				continue;
			} else {
				err_exit("epoll wait");
			}
		}

		for(i = 0; i < ready; i++) {

			if(evlist[i].events & EPOLLIN) {

				if(evlist[i].data.fd == server_fd) {
					client_len = sizeof(client_address);
					client_sockfd = accept(server_fd, (struct sockaddr *) &client_address, &client_len);
					if(-1 == client_sockfd) {
						log_err("accept failed");
						continue;
					}

					if(-1 == fds_add_client(client_sockfd)) {
						log_err("fds add client failed");
						continue;
					}

					struct epoll_event client_ev;
					client_ev.data.fd = client_sockfd;
					client_ev.events = EPOLLIN | EPOLLRDHUP;
					if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sockfd, &client_ev) == -1){
						log_err("epoll add failed");
						continue;
					}
					log_fmt_1("add client : %d", client_sockfd);

					// client connect or remote back
				} else {

					int nread;
					if(-1 == ioctl(evlist[i].data.fd, FIONREAD, &nread)) {
						log_fmt_err_1("get n read failed on fd : %d", evlist[i].data.fd);
						clean(evlist[i].data.fd, epoll_fd);
						continue;
					}
					if(nread == 0) {
						clean(evlist[i].data.fd, epoll_fd);
						log_fmt_1("close by fd : %d", evlist[i].data.fd);
						continue;
					}

					struct fds_pair temp_pair;
					int temp_is_client;
					if(-1 == fds_find_by_fd(evlist[i].data.fd, &temp_pair, &temp_is_client)) {
						log_fmt_err_1("fds find null : %d", evlist[i].data.fd);
						continue;
					}

					// client
					if(temp_is_client) {
						int new_remote_fd;
						switch(temp_pair.n_times) {
						case 0:
							if(-1 == connect1(temp_pair.client_fd)) {
								log_fmt_err_1("connect 1 failed on fd : %d", temp_pair.client_fd);
								clean(temp_pair.client_fd, epoll_fd);
							} else {
								fds_set_times(temp_pair.client_fd, 1);
								log_fmt_1("connect 1 success: %d", temp_pair.client_fd);
							}
							break;
						case 1:
							if(-1 == connect2(temp_pair.client_fd, &new_remote_fd)) {
								log_fmt_err_1("connect 2 failed on fd : %d", temp_pair.client_fd);
								clean(temp_pair.client_fd, epoll_fd);
							} else {
								fds_set_times(temp_pair.client_fd, 2);
								struct epoll_event ev_remote;
								ev_remote.data.fd = new_remote_fd;
								ev_remote.events = EPOLLOUT | EPOLLRDHUP;
								if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_remote_fd, &ev_remote) == -1){
									log_err("epoll add remote failed");
									continue;
								}
								log_fmt_1("connect 2 success: %d", temp_pair.client_fd);
							}
							break;

						default:
							if(trans(temp_pair.client_fd, temp_pair.remote_fd) == -1) {
								clean(temp_pair.client_fd, epoll_fd);
							}
							break;
						}

						// remote
					} else {
						if(trans(temp_pair.remote_fd, temp_pair.client_fd) == -1) {
							clean(temp_pair.client_fd, epoll_fd);
						}
					}

				}


			} else if(evlist[i].events & EPOLLOUT) {

				struct fds_pair write_pair;
				int write_is_client;
				if(-1 == fds_find_by_fd(evlist[i].data.fd, &write_pair, &write_is_client)) {
					log_fmt_err_1("find ready for write failed : %d", evlist[i].data.fd);
					clean(evlist[i].data.fd, epoll_fd);
					continue;
				}
				unsigned long ul0 = 0b11111111000000000000000000000000;
				unsigned long ul1 = 0b00000000111111110000000000000000;
				unsigned long ul2 = 0b00000000000000001111111100000000;
				unsigned long ul3 = 0b00000000000000000000000011111111;
				unsigned int up0 = 0b1111111100000000;
				unsigned int up1 = 0b11111111;
				unsigned char res[] = {
					5,
					0,
					0,
					1,
					(write_pair.ip & ul0) >> 24,
					(write_pair.ip & ul1) >> 16,
					(write_pair.ip & ul2) >> 8,
					write_pair.ip & ul3,
					(write_pair.port & up0) >> 8,
					write_pair.port & up1
				};
				if(-1 == write(write_pair.client_fd, res, sizeof(res))) {
					log_err("reply 2 failed");
					clean(write_pair.client_fd, epoll_fd);
				}
				struct epoll_event new_event;
				new_event.data.fd = write_pair.remote_fd;
				new_event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
				if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, evlist[i].data.fd, &new_event) == -1){
					log_err("epoll mod failed");
					clean(evlist[i].data.fd, epoll_fd);
				}
				continue;

			} else if((evlist[i].events & EPOLLERR) || (evlist[i].events & EPOLLHUP) || (evlist[i].events & EPOLLRDHUP)) {
				if(evlist[i].data.fd == server_fd) {
					err_exit("server fd failed");
				} else {
					log_fmt_err_1("fd error : %d", evlist[i].data.fd);
					clean(evlist[i].data.fd, epoll_fd);
				}
				continue;
			}

		}
	}

}

void clean(int fd, int epoll_fd) {
	struct epoll_event ev;
	struct fds_pair pair;
	int is_client;
	int other_fd;
	int res;

	if(-1 != fds_find_by_fd(fd,&pair, &is_client) ) {
		other_fd = (is_client ? pair.remote_fd : pair.client_fd);
		if(other_fd) {
			res = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, other_fd, &ev);
			close(other_fd);
		}
		fds_remove(fd);
	}

	res = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev);
	close(fd);
}

int connect1(int client_fd) {
	int nread;
	char buf[128];
	if(-1 == ioctl(client_fd, FIONREAD, &nread)) {
		log_err("connect 1 n read failed");
		return -1;
	}
	if(-1 == read(client_fd, buf, nread)) {
		log_err("connect 1 read failed");
		return -1;
	}

	if(buf[0] != 5) {
		log_err("un support proxy version");
		return -1;
	}

	char reply[] = {5, 0};
	return write(client_fd, reply, sizeof(reply));

}

int connect2(int client_fd, int *remote_fd) {
	int nread;
	unsigned char buf[512];
	if(-1 == ioctl(client_fd, FIONREAD, &nread)) {
		log_err("connect 2 n read failed");
		return -1;
	}
	if(nread > 512) {
		log_err("f k ");
		return -1;
	}
	if(-1 == read(client_fd, buf, nread)) {
		log_err("connect 2 read failed");
		return -1;
	}

	if(buf[0] != 5) {
		log_err("connect 2 : unsupport version ");
		return -1;
	}
	if(buf[1] != 1) {
		log_err("connect 2: not CONNECT");
		return -1;
	}

	unsigned long ip;
	unsigned int port;
	struct in_addr address;
	char host_name[512];
	int i;
	if(buf[3] == 1) {
		ip = ((unsigned long)buf[4] << 24) +
				((unsigned long)buf[5] << 16) +
				((unsigned long)buf[6] << 8) +
				buf[7];
		port = ((unsigned int)buf[8] << 8) + buf[9];
		address.s_addr = htonl(ip);

	} else if(buf[3] == 3) {
		for(i = 0; i< buf[4]; i++) {
			host_name[i] = buf[5+i];
		}
		host_name[i] = 0;
		log_fmt_1("host = %s", host_name);
		if(-1 == get_ip(host_name, &address)) {
			log_err("get ip failed");
			return -1;
		}
		port = ((unsigned int)buf[5 + buf[4]] << 8) + buf[6+buf[4]];

	} else {
		log_err("IP V6");
		return -1;
	}
	log_fmt_2("target == %s : %d", inet_ntoa(address), port);

	int other_fd;
	other_fd = connect_to(&address, port);
	if(other_fd == -1) {
		log_err("connect to failed");
		return -1;
	}

	if(fds_set_remote(client_fd, other_fd, address.s_addr, port) == -1) {
		log_err("fds set remote failed");
		return -1;
	}

	*remote_fd = other_fd;
	return 0;

}









