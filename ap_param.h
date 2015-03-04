#ifndef _AP_PARAM_H
#define _AP_PARAM_H


int init_blacklist(void); // should be called every time the AP restarts

int get_ap_mac(char * ap_mac);
int get_ap_status(unsigned char *p_buf, unsigned int length);
int get_ssid(unsigned char *p_buf);
int set_ssid(unsigned char *p_rev, unsigned char *p_buf);
int get_pppoe(unsigned char *p_buf);
int set_pppoe(unsigned char *p_rev, unsigned char *p_buf);
int get_static(unsigned char *p_buf);
int set_static(unsigned char *p_rev, unsigned char *p_buf);
int set_dhcp(unsigned char *p_buf);
int get_dhcp_cli_list_all(unsigned char *p_buf);
int set_restart(unsigned char * p_buf);
int set_add_blacklist(const char * mac);
int set_delete_balacklist(const char * mac);

#endif
