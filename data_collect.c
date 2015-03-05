#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <ctype.h>

#define G_IN 1
#define G_OUT 2
#define SERVICE_HOSTNAME_CZ "cz.lzyco.com"

#pragma pack(push, 1)
struct _header {
    int token;
    int type; // G_IN G_OUT
    int len;
};
struct _logout {
    time_t login;
    time_t logout;
    char g_mac[18]; // cellphone
    char m_mac[18]; // router
    unsigned long long outgoing;
    unsigned long long incomming;
};
#pragma pack(pop)

struct in_addr h_addr;
char router_mac[18] = "";

int mactoMAC(unsigned char* p_buf) {
    if(strlen(p_buf) != 17) {
        return -1;
    }
    int i;
    for (i=0; i < 17; i++) {
        *(p_buf+i) = toupper(*(p_buf+i)); 
    }
    return 0;
}

int read_dhcp_file(unsigned char* p_buf) {
    FILE* fp;
    char mac[18] = "";
    char ip[16] = "";
    char hostname[64] = "";
    char timein[11] = "";
    char str_tmp[128] = "";
    char tail[32] = "";

    fp = fopen("/tmp/dhcp.leases", "r");
    if (fp == NULL) {
        return -1;
    }
    fseek(fp, 0L, SEEK_SET);
    while(fscanf(fp, "%s%s%s%s%s\n", timein, mac, ip, hostname, tail) == 5) {
        mactoMAC(mac);
        printf("mac:%s;ip:%s;hostname:%s;timein:%s;\n", mac, ip, hostname, timein);
    }
    fclose(fp);
    return 0;
}

int get_dhcp_cli_list(unsigned char * p_buf) {

    char *p_buf_temp = p_buf;

    FILE *ping_result;
    
    char mac_tmp[18] = "";
    char ip_tmp[16] = "";
    char hostname_tmp[64] = "";
    char timein_tmp[11] = "";
    char str_tmp[128] = "";
    int time_diff;
	
    // read active client file
    ping_result = fopen("/tmp/ping_result", "r");
    if(NULL != ping_result)
    {   
        while(fgets(str_tmp, 128, ping_result) != NULL) // read each row
        {
            if (strlen(str_tmp) > 1)
            {
                sscanf(str_tmp, "%*[^:]:%[^;]%*[^:]:%[^;]%*[^:]:%[^;]%*[^:]:%[^;]",
                                           mac_tmp, ip_tmp, hostname_tmp, timein_tmp);
                sscanf(timein_tmp, "%d", &time_diff); 
                time_diff = (int)time(NULL) - time_diff;
                sprintf(p_buf_temp, "mac:%s;ip:%s;name:%s;type:%s;time:%d;status:%d;|",
                                         mac_tmp, ip_tmp, hostname_tmp, "",time_diff, 2);
                p_buf_temp += strlen(p_buf_temp);
            }
        }
        fclose(ping_result);
        return 0;
    }   

	// read dhcp list file
	if (read_dhcp_file(p_buf)) {
		return -1;
	}
	return 0;

}


int get_cli_logout(unsigned char* p_buf) {
    FILE *fp, *pingfp, *ping_result, *ping_result_tmp;
    char mac[18] = "";
    char ip[16] = "";
    char hostname[64] = "";
    char timein[11] = "";
    char str_tmp[128] = "";
    char tail[32] = "";
    char cmd[128] = "";
	unsigned char ping_status; // for debug
    char ping_result_str[128] = ""; // read file
    char* p_buf_temp = p_buf;

    fp = fopen("/tmp/dhcp.leases", "r");
    if (fp == NULL) {
        return -1;
    }

    fseek(fp, 0L, SEEK_SET);
    
    while(fscanf(fp, "%s%s%s%s%s\n", timein, mac, ip, hostname, tail) == 5) {
        mactoMAC(mac);
        // ping start
        sprintf(cmd, "ping %s -c 1 -w 1 | grep ttl > /tmp/pingtest", ip);
        system(cmd);
        pingfp = fopen("/tmp/pingtest", "r");
        
        if(NULL == pingfp) {
            return -1;
        }

        // debug
        printf("mac:%s;ip;%s", mac, ip);

		char ch;
        ch = fgetc(pingfp);
        if(ch == EOF) // not active
        {
            //debug
            ping_status = 0;
            printf(" status: %d\n", ping_status);

            ping_result = fopen("/tmp/ping_result", "r");
            // 1. file not exist. do nothing
            if(NULL != ping_result)
            {   
                ping_result_tmp = fopen("/tmp/ping_result_tmp", "w");
                if(NULL == ping_result_tmp) {
                    return -1;
                }
                while(fgets(ping_result_str, 128, ping_result) != NULL) // read each row
                {
                    if (strstr(ping_result_str, mac))// 2. file exist, mac is in the file
                    {
                        sprintf(p_buf_temp, "%stimeout:%ld;", ping_result_str, (long)time(NULL));
                        strcat(p_buf_temp, "|");
                        p_buf_temp += strlen(p_buf_temp);
                    }
                    else if (strlen(ping_result_str) != 1) // do not copy the empty line
                    {
                        fprintf(ping_result_tmp, "%s\n", ping_result_str);
                    }
                }
                fclose(ping_result_tmp);
                fclose(ping_result);
                system("rm /tmp/ping_result");
                system("cp /tmp/ping_result_tmp /tmp/ping_result");
                system("rm /tmp/ping_result_tmp");
            }
        } 
        else // active
        {
            //debug
            ping_status = 1;
            printf(" status: %d\n", ping_status);
                
            ping_result = fopen("/tmp/ping_result", "r");
            
            // 1. file not exist, create a new file and add current record.
            if(NULL == ping_result)
            {
                ping_result = fopen("/tmp/ping_result", "a");
                sprintf(ping_result_str, "mac:%s;ip:%s;hostname:%s;timein:%s;", mac, ip, hostname, timein);
                fprintf(ping_result, "%s\n", ping_result_str);
                fclose(ping_result);
                continue;
            }
                
            int find_currentmac = 0;
            // 2. file exist, mac is in the file
            while(fgets(ping_result_str, 128, ping_result) != NULL) // read each row
            {
                if (strstr(ping_result_str, mac)) // if mac in current row is equal to current mac, do nothing.
                {
                    find_currentmac = 1;
                    break;
                }   
            }
                
            // 3. file exist, mac not in the file, New client !!!
            if (find_currentmac == 0)
            {
                fclose(ping_result);
                ping_result = fopen("/tmp/ping_result", "a");
                sprintf(ping_result_str, "mac:%s;ip:%s;hostname:%s;timein:%s;", mac, ip, hostname, timein);
                fprintf(ping_result, "%s\n", ping_result_str);
            }
            fclose(ping_result);
        }
        fclose(pingfp);
    }
    fclose(fp);
    return 0;
}

int get_hostbyname_cz(void)
{
    struct hostent *h;
    struct in_addr *p_h_addr;
	static int flag_hostname = 0;

    if (0 == flag_hostname)
    {
	    if(NULL == (h = gethostbyname(SERVICE_HOSTNAME_CZ)))
        {
		    flag_hostname = 0;        
		    fprintf(stderr, "cannot get ip by hostname.");
      		return -1;	
        }
	    flag_hostname = 1;
	    h_addr = *(struct in_addr *)(h->h_addr);
        printf("connect db server, server ip = [%s].\n", inet_ntoa(h_addr));
    }
    return 0;
}

int dbsrv_send(int sock, int token, int type, void *data, int len)
{
    // header
    struct _header header;
    memset(&header, 0, sizeof(header));
    header.token = htonl(token);
    header.type = htonl(type);
    header.len = htonl(len);
    int length = len + sizeof(header);

    // info
    char *p = (char *)malloc(sizeof(char) * length);
    memset(p, 0, length);
    memcpy(p, &header, sizeof(header));
    memcpy(p+sizeof(header), data, len);

    write(sock, p, length);
    free(p);
    if (p != NULL)
        p = NULL;
    printf("dbsrv_send\n");
    return 0;
}

int get_router_mac(void)
{

    FILE* fp = popen("ifconfig | grep br-lan | awk '{print $5}'", "r");
    if(!fp) {
        return -1;
    }
    fgets(router_mac, 18, fp);
    pclose(fp);
    return 0;
	
}

int send_guestlogout(char *p_buffer)
{
    struct sockaddr_in s_add;
    unsigned short portnum = 5040; // server port
    char mac_tmp[18] = "";
    char ip_tmp[16] = "";
    char hostname_tmp[32] = "";
    char timein_tmp[11] = "";
    char timeout_tmp[11] = "";
    char buffer_tmp[128] = "";
    char *buffer_tmp_ptr = buffer_tmp;
    char *p_buffer_tmp = p_buffer;
    int time_tmp;
    struct _logout logout;

    // init socket
    bzero(&s_add, sizeof(struct sockaddr_in));
    s_add.sin_family = AF_INET;
    s_add.sin_addr.s_addr = h_addr.s_addr;
    //s_add.sin_addr.s_addr = inet_addr("192.168.1.200");
    s_add.sin_port = htons(portnum);
    
    if (!get_cli_logout(p_buffer_tmp)) // this function should be called in loop to update the active client file.
	{
        if (p_buffer_tmp != NULL) // some device log out.
        {
            //printf("Send to db server: %s\n", p_buffer_tmp);
                
            buffer_tmp_ptr = strtok(p_buffer_tmp, "|");
            while(buffer_tmp_ptr != NULL) // read each row.
            {
                //printf("One device info: %s\n", buffer_tmp_ptr);
                sscanf(buffer_tmp_ptr, "%*[^:]:%[^;]%*[^:]:%[^;]%*[^:]:%[^;]%*[^:]:%[^;]%*[^:]:%[^;]",
                                       mac_tmp, ip_tmp, hostname_tmp, timein_tmp, timeout_tmp);
                strcpy(logout.m_mac, router_mac);
                strcpy(logout.g_mac, mac_tmp);
                sscanf(timein_tmp, "%d", &time_tmp);
                logout.login = htonl((time_t)time_tmp);
                sscanf(timeout_tmp, "%d", &time_tmp);
                logout.logout = htonl((time_t)time_tmp);
                logout.incomming = 0L;
                logout.outgoing = 0L;
                
                // connect server
                int sock = socket(AF_INET, SOCK_STREAM, 0);
                if (-1 == connect(sock, (struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
                {
                    fprintf(stderr, "connect db server failed\n");
                    return -1;
                }
                // print struct
                printf("Send to server: m_mac:[%s];g_mac:[%s];timein:[%ld];timeout:[%ld]\n", logout.m_mac, logout.g_mac, logout.login, logout.logout);
                // send info to server.
                dbsrv_send(sock, 0x2014, G_OUT, (void *)&logout, sizeof(logout));
                close(sock);
                printf("Send success!\n");
                
                buffer_tmp_ptr = strtok(NULL, "|");
            }
                
            strcpy(p_buffer_tmp, ""); // clear buffer for next call.
            printf("Clean buffer: %s\n", p_buffer_tmp);
        }
    }
    else
    {
        fprintf(stderr, "call function get_cli_logout failed");
        return -1;
    }

    return 0;
}


void data_collect_init(void)
{
    if (get_router_mac())
    {
        fprintf(stderr, "cannot get router mac");
        return;
    }
    printf("%s", router_mac);
}

void data_collect_main(void)
{
	char buffer[1024] = ""; // store info to send to server.
    if (0 == get_hostbyname_cz())
    {
	    send_guestlogout(buffer);
    }
}
/*
int main()
{

    char buffer[1001]; // store info to send to server.
    if (get_hostbyname())
    {
        fprintf(stderr, "cannot get ip!\n"); // return to main
        return -1;
    }
    if (get_router_mac())
    {
        fprintf(stderr, "cannot get router mac");
        return -1;
    }
    printf("%s", router_mac);

    while(1)
	{

        //send_guestlogout(buffer);

        get_dhcp_cli_list(buffer);
        printf("\n%s\n", buffer);
        
        sleep(3);
	}
	return 0;
}*/
