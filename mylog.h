#ifndef __MY_LOG_H__
#define __MY_LOG_H__
enum{
	DEBUG,
	ERROR,
};
#define SIZE_M (1024*1024)
#define SIZE_K 1024

#ifndef LOG_FILE_MAX_SIZE_10M
//10M
//#define LOG_FILE_MAX_SIZE_10M 1024
#define LOG_FILE_MAX_SIZE_10M (SIZE_M*10)
#endif

#ifndef LOG_FILE_MAX_SIZE_5M
//5M
#define LOG_FILE_MAX_SIZE_5M (SIZE_M*5)
#endif
void log_init();
void log_delete();
void log_write(int level,const char*file,int line,const char *fmt,...);

#define log_stderr(fmt,...) log_write(ERROR,__FILE__,__LINE__,fmt,##__VA_ARGS__)
#define log_stdout(fmt,...) log_write(DEBUG,__FILE__,__LINE__,fmt,##__VA_ARGS__)
extern int g_max_log_size;
#endif
