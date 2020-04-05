#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>

#include "mysock.h"
#include "myredis.h"
#include "public.h"
#include "mylog.h"

#include "cJSON.h"
#define ITEM(r,v) cJSON_GetObjectItem((r),(v))
int openUdpBindSocketIpv4(int port)
{
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0 ){
		return -1;
	}
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	bzero(&addr,addrlen);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
//	int val = 1;
//	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));	
	if(bind(sock,(struct sockaddr*)&addr,addrlen) < 0){
		close(sock);
		return -1;
	}
	return sock;
 }




static char * my_mem_copy(char *dest,int size,char *src){
	int len = strlen(src);
	if( len > size)
		return dest;
	memset(dest,0,size);
	memcpy(dest,src,strlen(src));
	return dest;
}

static void value_string_update(cJSON *item,char *pbuf,int size){
	if(item && item->type == cJSON_String){
		my_mem_copy(pbuf,size,item->valuestring);
	}	
}

void value_bool_update(cJSON *item,int *pnum){
	if(item && item->type == cJSON_String){
		if(strcmp(item->valuestring,"true") == 0)
			*pnum = true;
	}		
}

static void value_int_update(cJSON *item,int *pnum){
	if(item && item->type == cJSON_Number){
		*pnum = item->valueint;
	}		
}

static bool check_item(RecvDataItems_t *item){
	if(strlen(item->type) == 0)
		return false;
	if(strlen(item->ssid) == 0)
		return false;
	if(item->pid == 0)
		return false;
	if(item->height == 0)
		return false;
	if(item->width == 0)
		return false;
	if(item->bpp == 0)
		return false;
	return true;
}

static int parse_recv_data(const char *buffer,RecvDataItems_t *data_item){
	cJSON *root = cJSON_Parse(buffer);
	if(root == NULL)
		return false;
	value_string_update(ITEM(root,"type"),data_item->type,sizeof(data_item->type));
	value_string_update(ITEM(root,"ssid"),data_item->ssid,sizeof(data_item->ssid));
	value_int_update(ITEM(root,"width"),&data_item->width);
	value_int_update(ITEM(root,"height"),&data_item->height);
	value_int_update(ITEM(root,"bpp"),&data_item->bpp);
	value_int_update(ITEM(root,"pid"),&data_item->pid);

	cJSON_Delete(root);
	if(!check_item(data_item)){
		return false;
	}
	return true;
}

void sendJSONpakcet(int fd,struct sockaddr_in *addr){
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"hostname","192.168.0.102");
	cJSON_AddStringToObject(root,"username","administrator");
	cJSON_AddStringToObject(root,"password","000000");		
	cJSON_AddStringToObject(root,"port","3389");
	cJSON_AddStringToObject(root,"protocol","rdp");
	cJSON_AddStringToObject(root,"connect_path","/tmp/");
	cJSON_AddStringToObject(root,"domain","");
	cJSON_AddStringToObject(root,"initial-program","");
	cJSON_AddStringToObject(root,"security","tls");
	cJSON_AddStringToObject(root,"enable-drive","true");
	cJSON_AddStringToObject(root,"upside_clip_flag","true");
	cJSON_AddStringToObject(root,"downside_clip_flag","true");
	cJSON_AddStringToObject(root,"file_upside_clip_flag","true");
	cJSON_AddStringToObject(root,"file_downside_clip_flag","true");
	cJSON_AddStringToObject(root,"console","");
	cJSON_AddStringToObject(root,"mstsc","true");
	char *buffer = cJSON_Print(root);
	int len = strlen(buffer);
	sendto(fd,buffer,len,0,(struct sockaddr*)addr,sizeof(struct sockaddr_in));
	cJSON_free(buffer);
	cJSON_Delete(root);
}



static char g_buffer[2048];

void backtoclient(int sock,void *addr,const char *buf,int len){
	log_stdout("send:%s\n",buf);
	if(sendto(sock,buf,len,0,(struct sockaddr*)addr,\
		sizeof(struct sockaddr_in)) < 0){
		perror("sendto");
	}
}


void publish_data_forward_java(redisContext *context,const char *localip,const RecvDataItems_t *item){
	snprintf(g_buffer,sizeof(g_buffer),\
		"publish session_serverIp [%s][%s][%d][%d][%d][%d]",\
		item->ssid,localip,item->width,\
		item->height,item->bpp,item->pid);
	redisReply *reply = redisCommand(context,g_buffer);
	if(reply == NULL){
		log_stderr("reply==NULL");
		return ;
	}
	if(reply->type == REDIS_REPLY_ERROR){
		log_stderr("Failed execute command:%s\n",g_buffer);
		log_stderr("Error:%s\n",reply->str);
	}else{
		log_stdout("Successed command:\n%s",g_buffer);
	}
	freeReplyObject(reply);
}


void YHXX(const inter_t *self,const RecvDataItems_t *item)
{
	const char *password = NULL;
	int i = 0;
	if(g_strcmp(item->type,"YHXX")){
		if(self->config->nums_cluster == 0){
			log_stderr("config for redis address error!");
			backtoclient(self->sock,self->addr_cli,"{}",g_strlen("{}"));
			return ;
		}
		for(i=0;i<self->config->nums_cluster;++i){
			if(self->config->cluster[i].valid == 0)
				continue;
			if(g_strlen(self->config->cluster[i].password)==0){
				password = self->config->global.password;
			}
			else {
				password = self->config->cluster[i].password;
			}
			const char *ipaddr = self->config->cluster[i].ipaddr;
			const char *port = self->config->cluster[i].port;
			redisContext *context = myredis_auth_connect(ipaddr,atoi(port),password);
			if(context == NULL){
				log_stdout("auth_connect failed:%s,%s,%s",ipaddr,port,password);
				continue;
			}
			memset(g_buffer,0,sizeof(g_buffer));
			if(myredis_get_session(context,item->ssid,\
				g_buffer,sizeof(g_buffer)) > 0){
				//返回信息
				backtoclient(self->sock,self->addr_cli,\
					g_buffer,g_strlen(g_buffer));
				//发布消息
				publish_data_forward_java(context,\
					self->config->global.localIp,item);
				redisFree(context);
				break;//只要发布成功就结束loop
			}else {
				//反馈失败信息
				backtoclient(self->sock,self->addr_cli,\
					"{}",g_strlen("{}"));
				redisFree(context);
				break;
			}
			redisFree(context);	
		}
	}
}

void send_server_quit(void){
	struct  sockaddr_in addr;
	bzero(&addr,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0){
		perror("socket");
		exit(0);
	}
	sendto(sock,"%EXIT%",6,0,(struct sockaddr*)&addr,sizeof(addr));
	close(sock);
}

int readUdpSocket(inter_t *inter,int sock){
	 RecvDataItems_t rdi;
	 char buffer[MAX_BUFFER_LEN];
	 struct sockaddr_in addr_cli;
	 socklen_t addrlen = sizeof(addr_cli);
	 bzero(buffer,sizeof(buffer));
	 bzero(&rdi,sizeof(rdi));
	 bzero(&addr_cli,addrlen);

	 inter->addr_cli = &addr_cli;
	 inter->sock = sock;
	 int nRecv = recvfrom(sock,buffer,MAX_BUFFER_LEN,0,(struct sockaddr*)&addr_cli,&addrlen);
	 if(nRecv > 0){
#ifdef __DEBUG_OUT__	 	
		log_stdout("recv[%s:%d]:%s\n",inet_ntoa(addr_cli.sin_addr),\
		ntohs(addr_cli.sin_port),buffer);	 	
#endif
		if(g_strcmp(buffer,"%EXIT%")){
			return (-1);
		}
		if(parse_recv_data(buffer,&rdi)){
			YHXX(inter,&rdi);
		}
	 }
	 return 0;
 }

