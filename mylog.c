#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mylog.h"
int g_max_log_size = 0;
static int g_fd = -1;
#ifdef LOG_FILE_NAME
#undef LOG_FILE_NAME
#endif
#define LOG_FILE_NAME "/var/log/conn_server.log"

void log_init(){
	g_fd = open(LOG_FILE_NAME,O_RDWR|O_APPEND|O_CREAT,\
		S_IWUSR|S_IWGRP|S_IROTH);
	if(g_fd < 0){
		perror("open error");
		exit(1);
	}
}

void log_delete(){
	if(g_fd > 0)
		close(g_fd);
}

void log_check_size(){
	struct stat s;
	if(stat(LOG_FILE_NAME,&s) < 0){
		perror("stat");
		return;
	}
	if(g_max_log_size == 0)
		g_max_log_size = 10;//10M
	//printf("g_size:%d\n",g_max_log_size);
	if(s.st_size > (g_max_log_size*SIZE_M)){
		if(access(LOG_FILE_NAME,F_OK) == 0){
			unlink(LOG_FILE_NAME);
			log_delete();
			log_init();
		}
	}
}


/*
*本例不涉及多线程模式
*同步结构，因此不需要锁
*如果你要修改多线程模式，那就需要考虑重入性了
*/
void log_write(int level,const char*file,int line,const char *fmt,...){
	char out[2048];
	int nlen = 0;
	memset(out,0x00,2048);
	const char *levemsg = "DEBUG";
	if(level == ERROR){
		levemsg = "ERROR";
	}
	
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	
	nlen = snprintf(out,2048,"[%s][%04d-%02d-%02d %02d:%02d:%02d][%s:%d]:",\
	levemsg,tm->tm_year+1970,tm->tm_mon+1,tm->tm_mday,\
	tm->tm_hour,tm->tm_min,tm->tm_sec,file,line);

	va_list ap;
	va_start(ap,fmt);
	nlen+=vsnprintf(out+nlen,2048-nlen,fmt,ap);
	va_end(ap);

	if(nlen <= 0)
		return ;

	if(out[nlen-1] != '\n'){
		out[nlen++] = '\n';
		out[nlen] = 0x00;
	}
#ifdef __DEBUG_OUT__	
	if(level == ERROR)
		fprintf(stderr,"%s",out);
	else if(level == DEBUG){
		fprintf(stdout,"%s",out);
	}
#endif
	log_check_size();
	
	if(g_fd >= 0){
		write(g_fd,out,nlen);
	}	
}
