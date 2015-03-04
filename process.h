#ifndef _PROCESS_H
#define _PROCESS_H

#include <netinet/in.h>

#define SNAP_LEN 1518

#define SIZE_ETHERNET 14

#define ETHER_ADDR_LEN  6

/* Ethernet header */
struct dpi_ethernet {
	unsigned char  ether_dhost[ETHER_ADDR_LEN];     /* destination host address */
	unsigned char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
	unsigned short ether_type;                     /* IP? ARP? RARP? etc */
};

/* IP header */
struct dpi_ip {
	unsigned char  ip_vhl;                  /* version << 4 | header length >> 2 */
	unsigned char  ip_tos;                  /* type of service */
	unsigned short ip_len;                  /* total length */
	unsigned short ip_id;                   /* identification */
	unsigned short ip_off;                  /* fragment offset field */
#define IP_RF 0x8000                    /* reserved fragment flag */
#define IP_DF 0x4000                    /* dont fragment flag */
#define IP_MF 0x2000                    /* more fragments flag */
#define IP_OFFMASK 0x1fff               /* mask for fragmenting bits */
	unsigned char  ip_ttl;                  /* time to live */
	unsigned char  ip_p;                    /* protocol */
	unsigned short ip_sum;                  /* checksum */
	struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)
/* TCP header */
typedef unsigned int tcp_seq;

struct dpi_tcp {
	unsigned short th_sport;                /* source port */
	unsigned short th_dport;                /* destination port */
	tcp_seq th_seq;                         /* sequence number */
	tcp_seq th_ack;                         /* acknowledgement number */
	unsigned char  th_offx2;                /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
	unsigned char  th_flags;
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	unsigned short th_win;                  /* window */
	unsigned short th_check;                /* checksum */
	unsigned short th_urp;                  /* urgent pointer */
};
struct dpi_udp {
	unsigned short th_sport;
	unsigned short th_dport;
	unsigned short th_len;
	unsigned short th_check;
};
extern int get_mmac(void);
extern int do_dpi(const char *packet);

#endif
