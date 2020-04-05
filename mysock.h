#ifndef __MY_SOCK_H
#define __MY_SOCK_H
//#include "public.h"
struct RecvDataItems{
	char type[20];
	char ssid[256];
	int width;
	int height;
	int bpp;
	int pid;
};

typedef struct Stream{
	char *buf;
	int pos;
	int size;
}Stream_t;

struct Fds{
	int fd;
	Stream_t s; 
	void *(*handler)(void *);
};

typedef struct Config Config_t;

typedef struct inter{
	const char *data;
	int len;
	struct sockaddr_in *addr_cli;
	int sock;
	Config_t *config;
}inter_t;

typedef struct RecvDataItems RecvDataItems_t;

int openUdpBindSocketIpv4(int port);

int readUdpSocket(inter_t*,int sock);

void send_server_quit(void);

#endif
