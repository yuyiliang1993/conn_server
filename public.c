#include "public.h"
#include "mylog.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>


int g_strcmp(const char*s1,const char*s2){
	return (strcmp(s1,s2) == 0);
}

int g_strlen(const char *s){
	return strlen(s);
}


char *del_space(char*data){
	char *s = data;
	char *e = data+strlen(data)-1;
	while(s < e && isspace((int)*s)){
		++s;
	}
	while(e>s && isspace((int)*e)){
		*e = 0;
		--e;
	}
	return s;
}

int g_select_valid(int fd){
	if(fd < 0 || fd >= FD_SETSIZE)
		return 0;
	return 1;
}

int g_fd_can_recv(int fd, int ms)
{
	
    fd_set rfds;
    struct timeval time;
    int rv;
	if(!g_select_valid(fd)){
		return 0;
	}
	
	memset(&time, 0, sizeof(time));
	if(ms){
    	time.tv_sec = ms / 1000;
    	time.tv_usec = (ms * 1000) % 1000000;
	}
	FD_ZERO(&rfds);
    FD_SET(((unsigned int)fd), &rfds);
	rv = select(fd + 1, &rfds, 0, 0, &time);
	if( rv > 0){
		return 1;
    }
    return 0;
}


int g_fd_can_send(int fd, int ms)
{
    fd_set wfds;
    struct timeval time;
	if(!g_select_valid(fd)){
		return 0;
	}
	memset(&time, 0, sizeof(time));
	if(ms){
    	time.tv_sec = ms / 1000;
    	time.tv_usec = (ms * 1000) % 1000000;
	}
	FD_ZERO(&wfds);
    FD_SET(((unsigned int)fd), &wfds);
    if(select(fd + 1, 0, &wfds, 0, &time) > 0){
		return 1;
    }
    return 0;
}

ConnRedisInfo_t get_connredisinfo(char *buf){
	ConnRedisInfo_t conn;
	bzero(&conn,sizeof(conn));
	char *p = strtok(buf,":");
	int k = 0;
	conn.valid = 0;
	for(;p!=NULL;p=strtok(NULL,":")){
		p = del_space(p);
		if(k == 0){
			snprintf(conn.ipaddr,sizeof(conn.ipaddr),"%s",p);
		}
		if(k == 1){
			snprintf(conn.port,sizeof(conn.port),"%s",p);
		}
		if(k == 2){
			snprintf(conn.password,sizeof(conn.password),"%s",p);
		}
		++k;
	}
	if(k >= 2)
		conn.valid = 1;
	return conn;
}

int config_read_file(Config_t *config,const char *filename){
	if(config==NULL || filename == NULL)
		return -1;
	bzero(config,sizeof(Config_t));
	config->nums_cluster = 0;
	FILE *fp = fopen(filename,"r");
	if(fp == NULL){
		perror("fopen");
		return -1;
	}
	char buf[1024];
	int section = 0;
	while(1){
		memset(buf,0x00,sizeof(buf));
		if(fgets(buf,1024,fp) == NULL){
			if(feof(fp))
				break;
			else
				continue;
		}
		if(buf[0] == '#' ||buf[0] == '\n' || \
			buf[0] == '\r' || buf[0] == ' ')
			continue;
		char *p = del_space(buf);
		if(strcmp(p,"[global]") == 0){
			section = 1;
			continue;
		}
		if(strcmp(p,"[cluster]") == 0){
			section = 2;
			continue;
		}
		if(section == 1){
			if(strncasecmp(p,"localip",7) == 0){
				char *str = strstr(p,"=");
				if(str){
					++str;
					snprintf(config->global.localIp,256,"%s",del_space(str));
				}
				
			}
			if(strncasecmp(p,"password",8) == 0){
				char *str = strstr(p,"=");
				if(str){
					++str;
					snprintf(config->global.password,256,"%s",\
						del_space(str));
				}
			}
			if(strncasecmp(p,"max_log_size",12) == 0){
				char *str = strstr(p,"=");
				if(str){
					++str;
					snprintf(config->global.max_log_size,32,"%s",\
						del_space(str));
				}
			}
						
		}else if(section == 2){
			ConnRedisInfo_t conn = get_connredisinfo(p);
			if(conn.valid){
				config->cluster[config->nums_cluster++] = conn;
			}
		}
	}
	fclose(fp);
	return 0;
}

void config_show(Config_t *config){
	log_stdout("password:%s",config->global.password);
	log_stdout("localip:%s",config->global.localIp);
	int i = 0;
	log_stdout("cluster config:\n");
	for(;i<config->nums_cluster;++i){
		if(config->cluster[i].valid){
			log_stdout("ipaddr:%s",config->cluster[i].ipaddr);
			log_stdout("port:%s",config->cluster[i].port);
			log_stdout("password:%s",config->cluster[i].password);
			log_stdout("\n");
		}
	}
}

