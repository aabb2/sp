#include <unistd.h>
#include <sys/ioctl.h>
#include "sp.h"

int trans(int from_fd, int to_fd) {
	const int BUF_SIZE = 2048;
	int result;
	int total;
	int nread;
	char buf[BUF_SIZE];
	ioctl(from_fd, FIONREAD, &total);

	do{
		nread = read(from_fd, buf, BUF_SIZE);
		if(nread == -1) return -1;
		if(-1 == write(to_fd, buf, nread)) return -1;
		total -= nread;
	}while(total > 0);

	return 0;
}
