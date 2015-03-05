/*************************************************************************
	> File Name: tunnel_client.c
	> Author: 
	> Mail: 
	> Created Time: 2014年11月28日 星期五 10时09分06秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>
#include <resolv.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cmd_parse.h"
#include "ap_param.h"
#include "mips_dpi.h"
#include "netlink_user.h"


#define DEFAULT_DB_PORT 8011
#define MAXBUF 1024
#define SERVICE_HOSTNAME "ws.lzyco.com"

/*****************parameters declare***************************/
struct sockaddr_in srv_addr;
struct in_addr h_addr;
int c_fd = 0;
unsigned char rev_buf[MAXBUF] = "";
unsigned char send_buf[MAXBUF] = "";
unsigned char mac_addr[18] = "";


/*****************function declare***************************/
int srv_init(void);
void socket_handle(void);
void socket_handle1(void);
int get_hostbyname(void);
int get_mac_addr(int sock);
int send_datas(unsigned short cmd, unsigned short count, unsigned char *p_datas);
void create_thread();
void create_process(void);

/*****************extern function***************************/
extern int cmd_handle(unsigned char *p_buf);
extern void data_collect_init(void);
extern void data_collect_main(void);
extern int init_mips_dpi(void);
extern int init_netlink(void);
extern void main_netlink(void);


/*****************************************************************/
int main()
{
    //unsigned char buf[1000];
    //unsigned char buf1[1000];

	printf("debug\n");

//	get_hostbyname();
    printf("LZY openwrt AP\n");


	//pc_firewall_init();
    //socket_handle();
    
    create_process();


    //socket_handle1();

    /*get_ap_status(buf, sizeof(buf));
    get_ssid(buf);
    get_pppoe(buf);
    get_static(buf);
    set_ssid("ssid:wdkwdk;ssidpwd:1234567890abc;", buf);
    get_ssid(buf);*/
    /*
     printf("dhcp client:[%d]\n", get_dhcp_cli_count());

     get_dhcp_cli_list(buf);
     printf("%s\n", buf);

    unsigned char mac_addr[100] = "mac:18:cf:5e:c4:e3:d4;aa:bb;";
    sscanf(mac_addr, "%*[^:]:%[^;]%*[^:]:%[^;]", buf, buf1);
    printf("mac:[%s];aa:[%s];", buf, buf1);*/
    return 0;
}


/***********************function********************************/
int srv_init(void)
{
    printf("srv init socket \n");
    c_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == c_fd)
    {
        return -1;
    }
    get_mac_addr(c_fd);
    
    srv_addr.sin_family = AF_INET;


	// debug using cellphone
    //srv_addr.sin_addr.s_addr = inet_addr("192.168.10.244");
    // send by hostname
    srv_addr.sin_addr.s_addr = h_addr.s_addr;//inet_addr("192.168.1.200");//inet_addr("203.100.95.160");//("210.14.151.119");//("192.168.1.113");//
	//
	//srv_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));
	srv_addr.sin_port = htons(DEFAULT_DB_PORT);
    
    printf("srv init connect\n");
    if (-1 == connect(c_fd, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr)))
    {
        close(c_fd);
        printf("connect error: %s\n", strerror(errno));
        return -1;
    }
    
    printf("get mac addr \n");
    send_datas(CMD_GET_MAC, (unsigned short)strlen(mac_addr), mac_addr);
    //write(c_fd, mac_addr, strlen(mac_addr));

    return 0;
}


void socket_handle1(void)
{   
    int rev_length = 0;
    int re = 0;
    int sockfd, len;
    //struct sockaddr_in dest;
    //char buffer[MAXBUF + 1];
    fd_set rfds;
    struct timeval tv;
    int retval, maxfd = -1;

    while(1)
    {
        printf("srv init handle1\n");
        re = srv_init();
        while(1)
        {
             /* clean the set */
            FD_ZERO(&rfds);
            maxfd = 0;
            
            /* add c_fd to set */
            FD_SET(c_fd, &rfds);
            if (c_fd > maxfd)
                maxfd = c_fd;
                
            /* set timeout */
            tv.tv_sec = 120;
            tv.tv_usec = 0;

            /* waiting data */
            retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
            if (retval == -1)
            {
                printf("select error: %s\n", strerror(errno));
                break;
            } 
            else if (retval == 0) 
            {
                printf("timeout! no message! continue...\n"); 
                ///continue;
                break;
            }
            else
            {
                if (FD_ISSET(c_fd, &rfds)) 
                {
                    /* clean rev buf */
                    bzero(rev_buf, MAXBUF + 1);
                    /*  */
                    len = recv(c_fd, rev_buf, MAXBUF, 0);
                    if (len > 0)
                    {
                         //printf("\n", uffer, len);
                         cmd_handle(rev_buf);
                    }
                    else 
                    {
                        if (len < 0)
                            printf("'%s'\n", errno, strerror(errno));
                        else
                            printf("\n");
                        break;
                    }
                }
                printf("else !\n");
            }
        }

        close(c_fd);
        printf("client socket close!\n");
        sleep(10);
    }
}



void socket_handle(void)
{
    int rev_length = 0;
    int re = 0;

    while(1)
    {
        printf("srv init \n");
        re = srv_init();
        while(1)
        {
            rev_length = read(c_fd, rev_buf, 1024);
            if (-1 == rev_length)
            {
                printf("read error -1\n");
                break;
            }
            else if (0 == rev_length)
            {
                printf("read error 0 \n");
                break;
            }
            else if (rev_length >= (4 + 17))
            {
                cmd_handle(rev_buf);
            }
            else
            {;}

            printf("data %d:%s\n", rev_length, rev_buf);
        }

        close(c_fd);
        sleep(10);
    }
}


int get_hostbyname(void)
{   
    
    struct hostent *h;
    struct in_addr *p_h_addr;

    while(1)
    {
        if((h = gethostbyname(SERVICE_HOSTNAME)) == NULL)
        {
            //fprintf(stderr, "get hostbyname error\n");
            //return -1;
            printf("get hostname error!\n");
            sleep(10);
            continue;
        }
        h_addr = *(struct in_addr *)(h->h_addr);

        printf("server ip=[%s];\n", inet_ntoa(h_addr));
        //memcpy((unsigned char *)&h_addr, h->h_addr, sizeof(struct in_addr));
        break;
    }
    return 0;
}


// get self mac
int get_mac_addr(int sock)
{ 
/*    struct ifreq ifreq; 
    char *p_mac;

    strcpy(ifreq.ifr_name,"br-lan");
    if (ioctl(sock,SIOCGIFHWADDR, &ifreq) <0)
    {
        perror( "ioctl ");
        printf("ioctl error\n");
        return -2;
    }

    sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X", 
            (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
    printf("mac [%d]: %s\n", (int)strlen(mac_addr), mac_addr);

*/

    FILE* fp = popen("ifconfig | grep br-lan | awk '{print $5}'", "r");
    if(!fp) {
        return -1;
    }
    fgets(mac_addr, 18, fp);
    pclose(fp);
	printf("mac: %s\n", mac_addr);
	
    return 0;
}


int send_datas(unsigned short cmd, unsigned short count, unsigned char *p_datas)
{
    unsigned char *p_send_buf = (unsigned char *)send_buf;

    /* cmd + mac + length + datas */
    *(unsigned short *)p_send_buf = htons(cmd);//cmd
    p_send_buf += 2;
    memcpy((unsigned char *)p_send_buf, mac_addr, count);//mac
    p_send_buf += strlen(mac_addr);
    *(unsigned short *)p_send_buf = htons(count);//length
    p_send_buf += 2;
    
    //memcpy((unsigned char *)p_send_buf, p_datas, count);//datas
    strcpy((unsigned char *)p_send_buf, p_datas);//datas

    printf("send [%d][%d]:[%s]\n", cmd, count, p_send_buf);
    //return write(c_fd, send_buf, count + 4 + strlen(mac_addr));
    return send(c_fd, send_buf, count + 4 + strlen(mac_addr), 0);
}


/*********************thread***************************************/
/*pthread_t tid;

void *thrd_func(void *arg){
    printf("I am new thread!\n");

	init_netlink();

	while(1)
    {
        //printf("New process:  PID: %d,TID: %u.\n",getpid(),pthread_self()); //why pthread_self
        //printf("New process:  PID: %d,TID: %u.\n",getpid(),tid); //why pthread_self
        printf("thread run again.\n");

		//TODO
		//periodicListRun(1);
		//init_mips_dpi();
		
		main_netlink();
		
        sleep(60*5);
    }
    pthread_exit(NULL); 
}
void create_thread()
{
    if (pthread_create(&tid,NULL,thrd_func,NULL)!=0) 
    {
        printf("Create thread error!\n");
        exit(1);
    }
    printf("Create thread success!\n");
}
*/
/*********************fork***************************************/


void create_process(void)
{
    int sign = 0x0F;
    pid_t pc1, pr1, pc2, pr2, pc3, pr3, pc4, pr4;

    while (1)
    {        
        if ((sign&0x01) == 0x01)
        {
            sign &= ~0x01;
            printf("create process 1!\n");
            pc1 = fork();
            if (pc1 < 0) 
				printf("Error occured on forking 1.\n");
            else if (pc1 == 0)
            {
      //          socket_handle1();
                exit(0);
            }
        }
        if ((sign&0x02) == 0x02)
        {
            sign &= ~0x02;
            printf("create process 2!\n");
            pc2 = fork();
            if (pc2 < 0) 
                printf("Error occured on forking 2.\n");
            else if (pc2 == 0)
            { 
                while(1) {
					init_netlink();
                    main_netlink();
                    sleep(5);
                }
                exit(0);    
            }
        }
        if ((sign&0x04) == 0x04)
        {
            sign &= ~0x04;
            printf("create process 3!\n");
            pc3 = fork();
            if (pc3 < 0) 
                printf("Error occured on forking 3.\n");
            else if (pc3 == 0)
            { 
      //          init_mips_dpi();
                exit(0);
            }
        } 

        if ((sign&0x08) == 0x08)
        {
            sign &= ~0x08;
            printf("create process 4!\n");
            pc4 = fork();
            if (pc4 < 0) 
                printf("Error occured on forking 4.\n");
            else if (pc4 == 0)
            { 
      //          data_collect_init();
				while(1) {
       //             data_collect_main();
                    sleep(2);
                }
                exit(0);
            }
        } 


        while(1)
        {
            sleep(60); // every 15 second do the check.

            pr1 = waitpid(pc1, NULL, WNOHANG);
            pr2 = waitpid(pc2, NULL, WNOHANG);
            pr3 = waitpid(pc3, NULL, WNOHANG);
			pr4 = waitpid(pc3, NULL, WNOHANG);
            if (pr1 == 0 && pr2 == 0 && pr3 ==0 && pr4 ==0)
            { 
                printf("All process run OK \n");
                
            }
            else
            {
                if (pr1 == pc1)
                {
                    printf("process1 exit\n");
                    sign |= 0x01;
                }
                if (pr2 == pc2)
                {
                    printf("process2 exit\n");
                    sign |= 0x02;
                }
                if (pr3 == pc3)
                {
                    printf("process3 exit\n");
                    sign |= 0x04;
                }
				if (pr4 == pc4)
                {
                    printf("process4 exit\n");
                    sign |= 0x08;
                }
                if (sign > 0)
                {
                    printf("sign=[%d];\n", sign);
                    break;
                }
            }
        }
    }
}



/*
void create_process(void)
{
    int sign = 0x03;
    pid_t pc1, pr1, pc2, pr2;

    while (1)
    {        
        printf("fork\n");
        if ((sign&0x01) == 1)
        {
            sign &= ~0x01;
            printf("create process 1!\n");
            pc1 = fork();
            if (pc1 < 0) 
				printf("Error occured on forking 1.\n");
            else if (pc1 == 0)
            { 
                printf("child 1!\n");
                socket_handle1();
                exit(0);
            }
        }
        if ((sign&0x02) == 0x02)
        {
            sign &= ~0x02;
            printf("create process 2!\n");
            pc2 = fork();
            if (pc2 < 0) 
                printf("Error occured on forking 2.\n");
            else if (pc2 == 0)
            { 
                int i = 1;
                printf("child 22222!\n");
				//create_thread();
				init_mips_dpi();
				
				//pc_firewall_init();
                //data_collect_init();
              while(1)
                {
                    //periodicListRun(i);
                    i = 0;
                    sleep(1);
					

					
                    //data_collect_main();
                    //sleep(60 * 5);
                    sleep(5);
					
                }
                exit(0);    
            }
        }

        while(1)
        {
            pr1 = waitpid(pc1, NULL, WNOHANG);
            pr2 = waitpid(pc2, NULL, WNOHANG);
            if (pr1 == 0 && pr2 == 0)
            { 
                printf("No child exited pr1=[%d] pr2=[%d]\n", pr1, pr2);
                sleep(60);
            }
            else
            {
                sleep(60);
                printf("child exited pr1=[%d] pr2=[%d]\n", pr1, pr2);
                if (pr1 == pc1)
                {
                    sign |= 0x01;
                }
                if (pr2 == pc2)
                {
                    sign |= 0x02;
                }
                if (sign > 0)
                {
                    printf("sign=[%d];\n", sign);
                    break;
                }
            }
        }
    }
}
*/
