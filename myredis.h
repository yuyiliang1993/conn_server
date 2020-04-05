#ifndef __MY_REDIS_H
#define __MY_REDIS_H

#include "hiredis.h"

redisContext *myredis_auth_connect(const char*ipaddr,int port,const char*password);
int myredis_get_session(redisContext*context,const char*ssid,char *dest,int size);
#endif
