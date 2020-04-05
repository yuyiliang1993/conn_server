#ifndef __PUBLIC_H
#define __PUBLIC_H

#define  MAX_BUFFER_LEN 4096

#define XRDP_NET_CONF_FILE "/etc/xrdp/xrdp_network.conf"
#define SERVER_PORT 11190
#ifndef VERSION
#define VERSION ""
#endif

#ifndef __DEBUG__
#define __DEBUG__
#endif

typedef struct ConnRedisInfo{
	char ipaddr[128];
	char password[128];
	char port[6];//65535
	int valid;
}ConnRedisInfo_t;


struct Config{
	struct Global{
		char password[256];
		char localIp[50];
		char max_log_size[32];
	}global;
	ConnRedisInfo_t cluster[256];
	int nums_cluster;
};
typedef struct Config Config_t;

void config_show(Config_t *config);
int config_read_file(Config_t *config,const char *filename);

int g_strcmp(const char*s1,const char*s2);
int g_strlen(const char *s);

#endif