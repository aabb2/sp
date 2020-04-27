#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <string.h>
#include <unistd.h>
#include "log.h"

static FILE *p_info;
static FILE *p_err;

#define NAME_SIZE 512

/**
 * write message with times
 * */
static void write_with_time(FILE *f, char *msg) {
	char *time_str;
	time_t t;
	time(&t);
	time_str = ctime(&t);
	fwrite(time_str, sizeof(char), 24, f);
	fwrite(" ", sizeof(char), 1, f);
	fwrite(msg, sizeof(char), strlen(msg), f);
	fwrite("\n", sizeof(char), 1, f);
	fflush(f);
}

int log_init(char *dir) {
	char full_name[NAME_SIZE] = {0};

	char * file_name = "log.txt";
	strcat(full_name, dir);
	strcat(full_name, file_name);
	if(NULL == (p_info = fopen(full_name, "w+"))) {
		return -1;
	}

	memset(full_name, 0, NAME_SIZE);
	strcat(full_name, dir);
	char *err_name = "err.txt";
	strcat(full_name, err_name);
	if(NULL == (p_err = fopen(full_name, "w+"))) {
			return -1;
	}

	return 0;
}

void log_info(char *msg) {
	write_with_time(p_info, msg);
}


int log_info_time(char *msg) {
	struct tms t;
	clock_t clockTime;
	static long clockTicks = 0;

	if (clockTicks == 0) {  /* Fetch clock ticks on first call */
		clockTicks = sysconf(_SC_CLK_TCK);
		if (clockTicks == -1) {
			return -1;
		}

	}
	clockTime = clock();
	if (clockTime == -1)
		return -1;
	fwrite(msg, sizeof(char), strlen(msg), p_info);
	fprintf(p_info, " clock: %ld clocks-per-sec (%.2f ms)\n",
				(long) clockTime,
				(double) clockTime / CLOCKS_PER_SEC * 1000);
	if (times(&t) == -1)
		return -1;

	fprintf(p_info, " times() yields: user CPU=%.2f; system CPU: %.2f\n",
			(double) t.tms_utime / clockTicks,
			(double) t.tms_stime / clockTicks);
		return 0;
}

void log_err(char *msg) {
	write_with_time(p_err, msg);
}

int log_close() {
	if(-1 == fclose(p_info))
		return -1;
	return fclose(p_err);
}








