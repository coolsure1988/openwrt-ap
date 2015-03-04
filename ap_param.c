#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <net/route.h>
#include <sys/stat.h>
#include "ap_param.h"

// sub function start

int get_ap_mac(char* ap_mac) {
    FILE* fp = popen("ifconfig | grep br-lan | awk '{print $5}'", "r");
    if(!fp) {
        return -1;
    }
    fgets(ap_mac, 18, fp);
    pclose(fp);
    return 0;
}

int get_dhcp_cli_count(unsigned char *p_buf) {
    FILE *fp = popen("cat /tmp/dhcp.leases | wc -l", "r");
    if(!fp) {
        return -1;
    }
    fgets(p_buf, 4, fp);
    pclose(fp);
    
    // delete space
    int i = 0;
    while(i++ < strlen(p_buf)) {
        if(!isprint(*(p_buf+i)) || *(p_buf+i) == ' ') {
            *(p_buf+i) = '\0';
        }
    }
    
    return 0;
}

int set_wifi_on() {

	// uci set wireless.radio0.disabled=0
	// uci commit wireless
	// wifi


	system("ifconfig wlan0 down");
	
}

int set_wifi_off() {

	// uci set wireless.radio0.disabled=1
	// uci commit wireless
	
    system("ifconfig wlan0 up");
    system("/etc/init.d/network restart");
}

int get_ip(unsigned char* ifname, unsigned char* if_addr) {
    struct ifreq ifr;
    int skfd = 0;
    
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }
    strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
    if(ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
        close(skfd);
        return -1;
    }
    strcpy(if_addr, (const char* )inet_ntoa(((struct sockaddr_in* )&ifr.ifr_addr)->sin_addr));
    
    close(skfd);
    return 0;
}

int get_netmask(char* ifname, char* if_net) {
    struct ifreq ifr;
    int skfd = 0;

    if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }
    strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
    if(ioctl(skfd, SIOCGIFNETMASK, &ifr) < 0) {
        close(skfd);
        return -1;
    }
    strcpy(if_net, (char* )inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    close(skfd);
    return 0;
}

int get_wan_gateway(char *sgw)
{
	char   buff[256];
	int    nl = 0 ;
	struct in_addr dest;
	struct in_addr gw;
	int    flgs, ref, use, metric;
	unsigned long d,g,m;
	int    find_default_flag = 0;

	FILE *fp = fopen("/proc/net/route", "r");

	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (nl) {
			int ifl = 0;
			while (buff[ifl]!=' ' && buff[ifl]!='\t' && buff[ifl]!='\0')
				ifl++;
			buff[ifl]=0; 
			if (sscanf(buff+ifl+1, "%lx%lx%X%d%d%d%lx",
						&d, &g, &flgs, &ref, &use, &metric, &m)!=7) {
				fclose(fp);
				return -1;
			}

			if (flgs&RTF_UP) {
				dest.s_addr = d;
				gw.s_addr   = g;
				strcpy(sgw, (gw.s_addr==0 ? "" : (char *)inet_ntoa(gw)));

				if (dest.s_addr == 0) {
					find_default_flag = 1;
					break;
				}
			}
		}
		nl++;
	}
	fclose(fp);

    return 0;
}

int get_dns(int req, char *dns)
{
	FILE *fp;
	char buf[80] = {0}, ns_str[11];
	int type, idx = 0;

	fp = fopen("/etc/resolv.conf", "r");
	if (NULL == fp)
		return -1;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (strncmp(buf, "nameserver", 10) != 0)
			continue;
		sscanf(buf, "%s%s", ns_str, dns);
		idx++;
		if (idx == req)
			break;
	}
	fclose(fp);

	return 0;
}


// init blacklist, read the blacklist file, and add the MAC to iptables
int init_blacklist() {
    char strtmp[18] = "";
    char cmd[128] = "";

    FILE* fp = fopen("/etc/blacklist", "r");
    if(NULL == fp) {
        return 0;
    } 
    else {
        while(fgets(strtmp, 18, fp) != NULL) {
            if(strlen(strtmp) == 17) {
                sprintf(cmd, "iptables -L FORWARD | grep %s > /tmp/bl_test", strtmp);
                
                // DEBUG
                printf("%s\n", cmd);
                system(cmd);
                FILE* fp_tmp = fopen("/tmp/bl_test", "r");
                if (NULL == fp_tmp) {
                    return -1;
                }
                char ch = fgetc(fp_tmp);
                if(ch == EOF) {
                    printf("add %s\n", strtmp);
                    
                    // apply the iptable rule
                    sprintf(cmd, "iptables -A FORWARD -m mac --mac-source %s -j DROP", strtmp);
                    system(cmd);
                    continue;   
                }
                fclose(fp_tmp);
            }
        }
    }

    fclose(fp);
    
    return 0;
}

// sub function end


// 1 check whether this is LZY AP

// 2 get ap system info
int get_ap_status(unsigned char* p_buf, unsigned int length) {
    // mac
    unsigned char mac[18] = "";
    if(get_ap_mac(mac)) {
        return -1;
    }
    
    // version
    unsigned char version[8] = "";
    FILE* fp = popen("uname -r", "r");
    if(!fp) {
        return -1;
    }
    fgets(version, 8, fp);
    char* version_p = version;
    if(version_p + strlen(version_p)  != NULL) {
        *(version_p + strlen(version_p) ) = '\0';
    }

    // wlan
    unsigned char wlan[16] = "";
    fp = popen("ifconfig eth0 | grep 'inet addr' | sed 's/^.*addr://g' | sed 's/Bcast.*$//g'", "r");
    if(!fp) {
        return -1;
    } 
    fgets(wlan, 16, fp);
    // delete space
    char* wlan_p = wlan;
    int i = 0;
    while(i++ < strlen(wlan_p)) {
        if(!isprint(*(wlan_p+i)) || *(wlan_p+i) == ' ') {
            *(wlan_p+i) = '\0';
        }
    }

    // personCount
    unsigned char cli_num[4] = "";
    if(get_dhcp_cli_count(cli_num)) {
        return -1;
    }

    // concatenat string
    sprintf(p_buf, "mac:%s;version:%s;wlan:%s;personCount:%s;speed:%s;status:%d;msg:%s;",
            mac, version_p, wlan_p, cli_num, "", 0, "");

    pclose(fp);
    return 0;
}

// 3 get ap wireless setting
int get_ssid(unsigned char *p_buf) {
    unsigned char ssid[128] = "";
    unsigned char ssidpwd[128] = "";
    FILE *fp = popen("uci get wireless.@wifi-iface[0].ssid", "r");
    if(!fp) {
        return -1;
    }
    fgets(ssid, 128, fp);
    fp = popen("uci get wireless.@wifi-iface[0].key", "r");
    if(!fp) {
        return -1;
    }
    fgets(ssidpwd, 128, fp);
    pclose(fp);
    
    sprintf(p_buf, "ssid:%s;ssidpwd:%s;status:%d;msg:%s;", 
            ssid, ssidpwd, 0, "");
    
    return 0;
}

// 4 set ap wireless
int set_ssid(unsigned char* p_rev, unsigned char* p_buf) {
    unsigned char ssid[128] = "";
    unsigned char ssidpwd[128] = "";
    unsigned char cmd[256] = "";
    
    sscanf(p_rev, "%*[^:]:%[^;]%*[^:]:%[^:]", ssid, ssidpwd);
    
    system("uci set wireless.@wifi-iface[0].encryption=psk2");
    sprintf(cmd, "uci set wireless.@wifi-iface[0].ssid=%s", ssid);
    system(cmd);
    sprintf(cmd, "uci set wireless.@wifi-iface[0].key=%s", ssidpwd);
    system(cmd);
    system("uci commit");
    system("/etc/init.d/network restart");

    sprintf(p_buf, "status:%d;msg:%s;", 0, "");
    return 0;
}


// 5 get pppoe setting
int get_pppoe(unsigned char *p_buf) {
    char name[128] = "";
    char pwd[128] = "";
    FILE* fp = popen("uci get network.wan.username", "r");
    if(!fp) {
        return -1;
    }
    fgets(name, 128, fp);
    fp = popen("uci get network.wan.password", "r");
    if(!fp) {
        return -1;
    }
    fgets(pwd, 128, fp);
    pclose(fp);
    
    sprintf(p_buf, "username:%s;password:%s;status:%d;msg:%s;",
           name, pwd, 0, "");
    return 0;
}

// 6\7 set pppoe
int set_pppoe(unsigned char* p_rev, unsigned char* p_buf) {
    unsigned char name[128] = "";
    unsigned char pwd[128] = "";
    unsigned char cmd[256] = "";
    
    sscanf(p_rev, "%*[^:]:%[^;]%*[^:]:%[^:]", name, pwd);
    system("uci set network.wan.proto=pppoe");
    sprintf(cmd, "uci set network.wan.username=%s", name);
    system(cmd);
    sprintf(cmd, "uci set network.wan.password=%s", pwd);
    system(cmd);
    system("uci commit");
    system("/etc/init.d/network restart");

    sprintf(p_buf, "status:%d;msg:%s;", 0, "");
    return 0;
}

// 8 get static setting
int get_static(unsigned char* p_buf) {
    unsigned char ip[16] = "";
    unsigned char netmask[16] = "";
    unsigned char gateway[16] = "";
    unsigned char priDNS[16] = "";
    unsigned char secDNS[16] = "";
    unsigned char cmd[256] = "";
   
    // call sub function
    get_ip("eth0", ip);
    get_netmask("eth0", netmask);
    get_wan_gateway(gateway);
    get_dns(1, priDNS);
    get_dns(2, secDNS);

    sprintf(p_buf, "ip:%s;subnetMask:%s;defaultGateway:%s;zhuDNS:%s;beiDNS:%s;status:%d:msg:%s;", ip, netmask, gateway, priDNS, secDNS, 2, "");
    
    return 0;
}
   

// 9\10 set static
int set_static(unsigned char* p_rev, unsigned char* p_buf) {
    unsigned char ip[16] = "";
    unsigned char netmask[16] = "";
    unsigned char gateway[16] = "";
    unsigned char priDNS[16] = "";
    unsigned char secDNS[16] = "";
    unsigned char cmd[256] = "";
    sscanf(p_rev, "%*[^:]:%[^;]%*[^:]:%[^;]%*[^:]:%[^;]%*[^:]:%[^;]%*[^:]:%[^;]", 
           ip, netmask, gateway, priDNS, secDNS);


	system("uci set network.wan.proto=static");
	sprintf(cmd, "uci set network.wan.ipaddr=%s", ip);
    system(cmd);
    sprintf(cmd, "uci set network.wan.netmask=%s", netmask);
    system(cmd);
	sprintf(cmd, "uci set network.wan.gateway=%s", gateway);
    system(cmd);
	sprintf(cmd, "uci set network.wan.dns=%s", priDNS);
    system(cmd);
    system("uci commit");
    system("/etc/init.d/network restart");

    sprintf(p_buf, "status:%d;msg:%s;", 0, "");
    return 0;
    
}

// 11 get dhcp setting TODO
int get_dhcp(unsigned char* p_buf) {
     
}

// 12 set dhcp TODO
int set_dhcp(unsigned char* p_buf) {
    
}

// 13 restart ap
int set_restart(unsigned char* p_buf) {
    system("reboot");
    return 0;
}

// 14 get client list
int get_dhcp_cli_list_all(unsigned char* p_buf) {
    char strtmp[512] = "";
    char time_cli[10] = "";
    char mac[18] = "";
    char ip[16] = "";
    char name[128] = "";
    FILE *fp = popen("cat /tmp/dhcp.leases", "r");
    if(!fp) {
        return -1;
    }
    while(fgets(strtmp, 512, fp) != NULL)
    {
        sscanf(strtmp, "%s%s%s%s", time_cli, mac, ip, name);
        sprintf(p_buf, "mac:%s;ip:%s;name:%s;type:%s;time:%s;status:%d;|", 
                mac, ip, name, "",time_cli, 2);
        p_buf += strlen(p_buf);
    }
    fclose(fp);
    return 0;
}

// 17 add black list
int set_add_blacklist(const char* mac) {
    char strtmp[18] = "";
    char cmd[128] = "";

    // check whether the MAC is already in the file
    FILE* fp = fopen("/etc/blacklist", "r");
    if(NULL == fp) {
        system("touch /etc/blacklist");
    } 
    else {
        while(fgets(strtmp, 18, fp) != NULL) {
            if(strstr(strtmp, mac)) {
                // the MAC is in the file, do nothing.
                return 0;
            }
        }
    }

    // add the MAC to black list file
    fp = fopen("/etc/blacklist", "a");
    fprintf(fp, "%s\n", mac);
    
    // apply the iptable rule
    sprintf(cmd, "iptables -A FORWARD -m mac --mac-source %s -j DROP", mac);
    system(cmd);

    fclose(fp);

    return 0;   
}


// 18 delete black list item
int set_delete_balacklist(const char* mac) {
    char strtmp[18] = "";
    char cmd[128] = "";

    // check the MAC
    FILE* fp = fopen("/etc/blacklist", "r");
    if(NULL == fp) {
        return 0;
    }
    else {
        FILE* fp_tmp = fopen("/tmp/blacklist_tmp", "w");
        if(!fp_tmp) {
            return -1;
        }
        while(fgets(strtmp, 18, fp) != NULL) {
            
            if(strstr(strtmp, mac) || strlen(strtmp) == 1) {
                // do not copy the mac to be deleted
            }
            else {
                fprintf(fp_tmp, "%s\n", strtmp);
            }
        }
        fclose(fp);
        fclose(fp_tmp);
        
        system("rm /etc/blacklist");
        system("cp /tmp/blacklist_tmp /etc/blacklist");
        system("rm /tmp/blacklist_tmp");
    }

    // apply the iptable rule
    sprintf(cmd, "iptables -D FORWARD -m mac --mac-source %s -j DROP", mac);
    system(cmd);
    
    return 0;
}
