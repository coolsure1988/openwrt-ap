/*************************************************************************
	> File Name: cmd_parse.h
	> Author: 
	> Mail: 
	> Created Time: 2014年12月02日 星期二 10时28分45秒
 ************************************************************************/

#ifndef _CMD_PARSE_H
#define _CMD_PARSE_H

/***cmd***/
#define CMD_GET_MAC             0
#define CMD_GET_STAT            2
#define CMD_GET_SSID_CFG        3
#define CMD_SET_SSID_CFG        4
#define CMD_GET_PPPOE_CFG       5
#define CMD_ADD_PPPOE_CFG       6
#define CMD_SET_PPPOE_CFG       7
#define CMD_GET_STATIC_CFG      8
#define CMD_ADD_STATIC_CFG      9
#define CMD_SET_STATIC_CFG      10
#define CMD_SET_DHCP_CFG        12
#define CMD_REBOOT              13
#define CMD_GET_DCHP_CLI_LIST   14

#define CMD_BLACK_LIST_ADD      17
#define CMD_BLACK_LIST_DEL      18

#define CMD_PERIODIC_LIST_GET   20
#define CMD_PERIODIC_LIST_ADD   21
#define CMD_PERIODIC_LIST_DEL   22
#define CMD_PERIODIC_LIST_FLUSH 23

#define CMD_RADIO_POWER_ADD     24
#define CMD_RADIO_POWER_SET     25
#define CMD_RADIO_POWER_GET     27
#define CMD_GET_DEV_INFO        26
#define CMD_GET_WAN_MODE        28

#endif
