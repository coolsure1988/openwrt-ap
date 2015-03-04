#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <string.h>

#define NETLINK_TEST 31 // defined by user
#define RECIVE_BUF 4096 // recive buffer size 


struct sockaddr_nl dst_addr, src_addr;
struct nlmsghdr* nlh = NULL;
struct iovec iov;
int sock_fd = -1;
struct msghdr msg;

// construct message
//char str[] = "Message from user";
char str[500];



int init_netlink(void)
{

	// debug 
	printf("init_netlink\n");


    // create netlink socket
    if (-1 == (sock_fd = (socket(PF_NETLINK, SOCK_DGRAM, NETLINK_TEST)))) {
        perror("Can't create netlink socket!");
        return -1;
    }

    // for sending to kernel
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.nl_family = AF_NETLINK;
    dst_addr.nl_pid = 0;
    dst_addr.nl_groups = 0;

    // for reciving from kernel
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 0;
    bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));


    strcpy(str, "lzy");
    int len = strlen(str);
    printf("str len: %d\n", len);
    //if(NULL == (nlh = malloc(NLMSG_SPACE(len)))) {
    if(NULL == (nlh = malloc(NLMSG_SPACE(RECIVE_BUF)))) {
        perror("allocate memory failed\n");
    }
    //memset(nlh, 0, NLMSG_SPACE(len));
    
    memset(nlh, 0, NLMSG_SPACE(RECIVE_BUF));
    
    // fill the netlink header
    //nlh->nlmsg_len = NLMSG_SPACE(len)+sizeof(struct nlmsghdr);
    nlh->nlmsg_len = NLMSG_SPACE(RECIVE_BUF)+sizeof(struct nlmsghdr);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_type = 0;
    nlh->nlmsg_flags = 0;
    // netlink payload
    strcpy(NLMSG_DATA(nlh), str);
    // ...
    memset(&iov, 0, sizeof(iov));
    iov.iov_base = (void*) nlh;
    iov.iov_len = nlh->nlmsg_len;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void*) &dst_addr;
    msg.msg_namelen = sizeof(dst_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

	return 0;

}

void main_netlink(void) {

	// debug 
	printf("main_netlink\n");

    // send
    sendmsg(sock_fd, &msg, 0);
    free(nlh);
    
    // recive
    if(NULL == (nlh = malloc(NLMSG_SPACE(RECIVE_BUF)))) {
        perror("allocate memory failed\n");
    }
    memset(nlh, 0, RECIVE_BUF);
    recvmsg(sock_fd, &msg, 0);
    printf("Recive from kernel: %s\n", NLMSG_DATA(nlh));
    free(nlh);
    
    
    // close netlink socket
    close(sock_fd);

    return;
}


/*
int main(int argc, char* argv[])
{
    struct sockaddr_nl dst_addr, src_addr;
    struct nlmsghdr* nlh = NULL;
    struct iovec iov;
    int sock_fd = -1;
    struct msghdr msg;

    // create netlink socket
    if (-1 == (sock_fd = (socket(PF_NETLINK, SOCK_DGRAM, NETLINK_TEST)))) {
        perror("Can't create netlink socket!");
        return -1;
    }

    // for sending to kernel
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.nl_family = AF_NETLINK;
    dst_addr.nl_pid = 0;
    dst_addr.nl_groups = 0;

    // for reciving from kernel
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 0;
    bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

    // construct message
    //char str[] = "Message from user";
    char str[500];
    strcpy(str, argv[1]);
    int len = strlen(str);
    printf("str len: %d\n", len);
    //if(NULL == (nlh = malloc(NLMSG_SPACE(len)))) {
    if(NULL == (nlh = malloc(NLMSG_SPACE(RECIVE_BUF)))) {
        perror("allocate memory failed\n");
    }
    //memset(nlh, 0, NLMSG_SPACE(len));
    
    memset(nlh, 0, NLMSG_SPACE(RECIVE_BUF));
    
    // fill the netlink header
    //nlh->nlmsg_len = NLMSG_SPACE(len)+sizeof(struct nlmsghdr);
    nlh->nlmsg_len = NLMSG_SPACE(RECIVE_BUF)+sizeof(struct nlmsghdr);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_type = 0;
    nlh->nlmsg_flags = 0;
    // netlink payload
    strcpy(NLMSG_DATA(nlh), str);
    // ...
    memset(&iov, 0, sizeof(iov));
    iov.iov_base = (void*) nlh;
    iov.iov_len = nlh->nlmsg_len;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void*) &dst_addr;
    msg.msg_namelen = sizeof(dst_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // send
    sendmsg(sock_fd, &msg, 0);
    free(nlh);
    
    // recive
    if(NULL == (nlh = malloc(NLMSG_SPACE(RECIVE_BUF)))) {
        perror("allocate memory failed\n");
    }
    memset(nlh, 0, RECIVE_BUF);
    recvmsg(sock_fd, &msg, 0);
    printf("Recive from kernel: %s\n", NLMSG_DATA(nlh));
    free(nlh);
    
    
    // close netlink socket
    close(sock_fd);

    return 0;
}
*/
