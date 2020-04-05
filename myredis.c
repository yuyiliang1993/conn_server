
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "public.h"
#include "myredis.h"
#include "cJSON.h"
#include "mylog.h"

redisContext *myredis_auth_connect(const char*ipaddr,int port,const char*password){
	redisContext *context = redisConnect(ipaddr,port);
	if(context == NULL || context->err){
		if(context){
			log_stderr("%s\n", context->errstr);
		}else{
			log_stderr("redisConnect error\n");
		}
		return NULL;
	}
	int err = 0;
	redisReply *reply = redisCommand(context, "auth %s",password);
	if (reply && reply->type == REDIS_REPLY_ERROR) {
		log_stderr("auth failed:%s",reply->str);
		err = 1;
	}
	freeReplyObject(reply);
	if(err){
		redisFree(context);
		context = NULL;
	}	
	return context;
}

int myredis_get_session(redisContext*context,const char*ssid,char *dest,int size){
	if(context == NULL || ssid == NULL||dest == NULL|| size<=0){
		log_stderr("argments error");
		return -1;
	}
	
	char cmd[256];
	int len = 0;
	snprintf(cmd,256,"get connection_session::%s",ssid);
	log_stdout("command str:\n%s",cmd);
	redisReply *reply = redisCommand(context,cmd);
	if(reply && reply->type == REDIS_REPLY_STRING){
		const char *p = reply->str;
		if(reply->len <= 0){
			log_stderr("reply->len <= 0");
			freeReplyObject(reply);
			return (-1);	
		}
		while(p<reply->str+reply->len-1 && *p != '{')
			++p;
		if(*p != '{'){
			log_stderr("Redis data error:%s",reply->str);
			freeReplyObject(reply);
			return (-1);
		}
		len = strlen(p);
		if(len > size ){
			log_stderr("The dest buffer not enough space(the len:%d).",len);
			len = 0;
		}else{
			memcpy(dest,p,len);
		}
	}
	else if(reply){
		log_stdout("Failure replay not string:reply->len->%d,reply->type:%d\n",\
			reply->len,reply->type);
		len = 0;
	}
	freeReplyObject(reply);
	return len;
}
