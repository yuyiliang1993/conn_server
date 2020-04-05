
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>

//#include "myredis.h"
#include "mysock.h"
#include "public.h"
#include "mylog.h"

void server_main_loop(inter_t *inter,int sock)
{
	fd_set rfds;
	while(1){
		FD_ZERO(&rfds);
		FD_SET(sock,&rfds);
		if(select(sock+1,&rfds,NULL,NULL,NULL) < 0){
			if(errno == EINTR){
				continue;
			}
			log_stderr("select:%s",strerror(errno));
			break;
		}
		if(FD_ISSET(sock,&rfds)){
			if(readUdpSocket(inter,sock) < 0){
				log_stdout("Server get stop");
				break;
			}
		}
	}
}

void daemon_init(void)
{
	pid_t pid; 
	if((pid= fork()) < 0){
		perror("fork");
		exit(1);
	}
	
	if(pid > 0){
		exit(0);//父进程退出
	}
	setsid();//获取新得session
	chdir("/");//改变工作目录
	umask(0);//设置新的掩码
	int fd = open("/dev/null",O_RDWR);
	if(fd > 0){
		int i = 0;
		for(;i<3;++i){
			dup2(i,fd);
			close(i);
		}
	}
}

int main(int argc,char *argv[])
{
	//not use getopt
	if(argc>=2 && g_strcmp(argv[1],"-d")){
		daemon_init();
	}
	if(argc>=2 && g_strcmp(argv[1],"-s")){
		send_server_quit();
		exit(0);
	}
	if(argc>=2 && g_strcmp(argv[1],"-sd")){
		send_server_quit();
		daemon_init();
	}
	if(argc>=2 && g_strcmp(argv[1],"-v")){
		printf("vesion: %s\n",VERSION);
		return 0;
	}
	log_init();

	Config_t config;
	if(config_read_file(&config,XRDP_NET_CONF_FILE) < 0){
		log_stderr("Read config failed");
		exit(0);
	}
	if(g_strlen(config.global.max_log_size) > 0){
		int logsize = atoi(config.global.max_log_size);
		g_max_log_size = logsize;
	}
	log_stdout("g_max_log_size:%d\n",g_max_log_size);
	
	int sock = openUdpBindSocketIpv4(SERVER_PORT);
	if(sock < 0){
		log_stderr("openUdpBindSocketIpv4:%s",strerror(errno));
		log_delete();
		exit(1);
	}

	inter_t inter;
	inter.config = &config;
#ifdef __DEBUG_OUT__
	config_show(&config);	
	log_stdout("version: %s",VERSION);
	log_stdout("server starting:%d\n",SERVER_PORT);
#endif
	server_main_loop(&inter,sock);
	close(sock);	
	log_delete();
	return 0;
}

