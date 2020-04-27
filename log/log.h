#ifndef AB_LOG
#define AB_LOG

	int log_init(char *dir);
	void log_info(char *msg);
	void log_err(char *msg);
	int log_close();

	/*show process times*/
	int log_info_time(char *msg);

	#include <stdio.h>
	#include <unistd.h>

	static char log_fmt_buf[512];
	static int log_fmt_buf_n;
	#define log_fmt_1(A, B) log_fmt_buf_n = sprintf(log_fmt_buf, A, B); log_fmt_buf[log_fmt_buf_n] = '\0';\
							log_info(log_fmt_buf)
	#define log_fmt_2(A, B, C) log_fmt_buf_n = sprintf(log_fmt_buf, A, B, C); log_fmt_buf[log_fmt_buf_n] = '\0';\
							log_info(log_fmt_buf)

	#define log_fmt_err_1(A, B) log_fmt_buf_n = sprintf(log_fmt_buf, A, B); log_fmt_buf[log_fmt_buf_n] = '\0';\
							log_err(log_fmt_buf)
	#define log_fmt_err_2(A, B, C) log_fmt_buf_n = sprintf(log_fmt_buf, A, B, C); log_fmt_buf[log_fmt_buf_n] = '\0';\
							log_err(log_fmt_buf)


#endif
