/*************************************************************************
	> File Name: cmd_parse.c
	> Author: 
	> Mail: 
	> Created Time: 2014年12月02日 星期二 10时15分25秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/reboot.h>
#include <malloc.h>
#include "cmd_parse.h"
#include "ap_param.h"
#include "debug.h"

extern int c_fd;
extern char mac_addr[18];
extern int send_datas(unsigned short cmd, unsigned short count, unsigned char *p_datas);

/******************extern function*************************/

extern int get_ap_mac(char * ap_mac);
extern int get_ap_status(unsigned char *p_buf, unsigned int length);
extern int get_ssid(unsigned char *p_buf);
extern int set_ssid(unsigned char *p_rev, unsigned char *p_buf);
extern int get_pppoe(unsigned char *p_buf);
extern int set_pppoe(unsigned char *p_rev, unsigned char *p_buf);
extern int get_static(unsigned char *p_buf);
extern int set_static(unsigned char *p_rev, unsigned char *p_buf);
extern int set_dhcp(unsigned char *p_buf);
extern int get_dhcp_cli_list(unsigned char *p_buf);
extern int set_restart(unsigned char * p_buf);
extern int set_add_blacklist(const char * mac);
extern int set_delete_balacklist(const char * mac);

/*
*** command   ***
*/
int cmd_handle(unsigned char *p_buf)
{
    unsigned short cmd = 0;
    unsigned short count = 0;
    unsigned char buf[1024];
    
    cmd = ntohs(*(unsigned short *)p_buf);
    p_buf += 2;
    p_buf += 17;
    count = ntohs(*((unsigned short *)p_buf));
    p_buf += 2;
    
    #ifdef _DEBUG_H	
	    printf("rev [%d][%d]:[%s]\n", cmd, count, p_buf);
    #endif
    
    switch(cmd)
    {
        case CMD_GET_MAC:
            get_mac_addr(c_fd);
            send_datas(CMD_GET_MAC, (unsigned short)strlen(mac_addr), mac_addr);
        break;
        case CMD_GET_STAT:
        {
            get_ap_status(buf, sizeof(buf));
            send_datas(CMD_GET_STAT, (unsigned short)strlen(buf), buf);
            break;
        }
        case CMD_GET_SSID_CFG:
            
            get_ssid(buf);
            send_datas(CMD_GET_SSID_CFG, (unsigned short)strlen(buf), buf);
        break;
        case CMD_SET_SSID_CFG:
            set_ssid(p_buf, buf);
            send_datas(CMD_SET_SSID_CFG, (unsigned short)strlen(buf), buf);
        break;
        case CMD_GET_PPPOE_CFG:
            get_pppoe(buf);
            send_datas(CMD_GET_SSID_CFG, (unsigned short)strlen(buf), buf);
        break;
        case CMD_ADD_PPPOE_CFG:
        case CMD_SET_PPPOE_CFG:
            set_pppoe(p_buf, buf);
            send_datas(cmd, (unsigned short)strlen(buf), buf);
        break;
        case CMD_GET_STATIC_CFG:
            get_static(buf);
            send_datas(CMD_GET_SSID_CFG, (unsigned short)strlen(buf), buf);
        break;
        case CMD_ADD_STATIC_CFG:
        case CMD_SET_STATIC_CFG:
            set_static(p_buf, buf);
            send_datas(cmd, (unsigned short)strlen(buf), buf);
        break;
        case CMD_SET_DHCP_CFG:
            set_dhcp(buf);
            send_datas(cmd, (unsigned short)strlen(buf), buf);
        break;
        case CMD_REBOOT:
            sprintf(buf, "status:%d;msg:%s;", 0, "");
            send_datas(cmd, (unsigned short)strlen(buf), buf);
            sleep(1);
            reboot(LINUX_REBOOT_CMD_RESTART);
        break;
        case CMD_GET_DCHP_CLI_LIST:		
			get_dhcp_cli_list_all(buf);
			send_datas(cmd, (unsigned short)strlen(buf), buf);

		/*{

			unsigned char cli_num[4] = "";
			int dhcp_cli_count = 0;
			if(get_dhcp_cli_count(cli_num)) {
				return -1;
			}
			dhcp_cli_count = atoi(cli_num);
			int re = 0;
            unsigned char *p_data = NULL;
            
            if (dhcp_cli_count > 0)
            {
                p_data = (unsigned char *)malloc(dhcp_cli_count * 256);
                if (p_data != NULL)
                {
                    re = get_dhcp_cli_list(p_data);
                    
                    send_datas(cmd, (unsigned short)strlen(p_data), p_data);
                    free(p_data);
                }
                else
                {
                    sprintf(buf, "status:%d;msg:%s;", -1, "no memory.");
                    send_datas(cmd, (unsigned short)strlen(buf), buf);
                }
            }
            else
            {
                sprintf(buf, "status:%d;msg:%s;", 0, "");
                send_datas(cmd, (unsigned short)strlen(buf), buf);
            }
        }*/
        break;
        case CMD_BLACK_LIST_ADD:
        {
            unsigned char mac_addr[100] = "";
            sscanf(p_buf, "%*[^:]:%[^;]", mac_addr);
            #ifdef _DEBUG_H	
        	    printf("black list add:[%s]\n", mac_addr);
            #endif
            set_add_blacklist(mac_addr);
            sprintf(buf, "status:%d;msg:%s;", 0, "");
            send_datas(cmd, (unsigned short)strlen(buf), buf);
        }
        break;
        case CMD_BLACK_LIST_DEL:
        {
            unsigned char mac_addr[100] = "";
            sscanf(p_buf, "%*[^:]:%[^;]", mac_addr);
            #ifdef _DEBUG_H	
        	    printf("black list del:[%s]\n", mac_addr);
            #endif
            set_delete_balacklist(mac_addr);
            sprintf(buf, "status:%d;msg:%s;", 0, "");
            send_datas(cmd, (unsigned short)strlen(buf), buf);
        }
        break;
        case CMD_PERIODIC_LIST_GET:
        case CMD_PERIODIC_LIST_ADD:
        case CMD_PERIODIC_LIST_DEL:
        case CMD_PERIODIC_LIST_FLUSH:
        case CMD_RADIO_POWER_ADD:
        case CMD_RADIO_POWER_SET:
        case CMD_RADIO_POWER_GET:
        case CMD_GET_DEV_INFO:
        case CMD_GET_WAN_MODE:
        default:
            sprintf(buf, "status:%d;msg:%s;", 1, "none");
            send_datas(cmd, (unsigned short)strlen(buf), buf);
        break;
    }

    return 0;
}


