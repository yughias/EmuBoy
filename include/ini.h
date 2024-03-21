#ifndef __INI_H__
#define __INI_H__

#define CONFIG_STRING_SIZE 64

extern int config_listen_port;
extern int config_connect_port;
extern char config_connect_ip[CONFIG_STRING_SIZE];
extern char config_render[CONFIG_STRING_SIZE];
extern char config_gb_palette[CONFIG_STRING_SIZE];
extern int config_force_dmg_when_possible;

void loadConfigFile(const char* filename);

#endif