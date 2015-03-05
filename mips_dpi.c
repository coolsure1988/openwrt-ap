#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <pcap/pcap.h>
#include "process.h"
#include "report.h"

#include "mips_dpi.h"

// TODO move to conf file  mips-run.conf
// NIC name
#define NET_INTERFACE "br-lan"
// filter rule
#define FILTER_RULE "tcp port 80 and (((ip[2:2]-((ip[0]&0xf)<<2)) - ((tcp[12]&0xf0)>>2)) != 0)"
// default snap length (maximum bytes per packet to capture)
#define SNAP_LEN 1518

void do_packet(u_char *args, const struct pcap_pkthdr *pkthdr, const u_char *raw_pkt) {   
	report_conf_s *r_conf = 	(report_conf_s *)args;

	do_dpi(raw_pkt, r_conf);
    if(ready_to_send()) {
    	init_mqsock(r_conf);
    	send_msgs_to_mq(r_conf);
    	close_mqsock(r_conf);
		printf("get a bag\n");
    }   
    return;
}

int init_mips_dpi(void) {
    
    // init WQ
    report_conf_s *r_conf;
	r_conf = init_report();

    // init pcap;
    char error_content[PCAP_ERRBUF_SIZE]; //error buffer
    pcap_t *pcap_handle; // packet capture handle
    struct bpf_program fp; // filter
    bpf_u_int32 mask, net; // subnet mask and ip
    if (pcap_lookupnet(NET_INTERFACE, &net, &mask, error_content)) {
        fprintf(stderr, "Couldn't get netmask for device %s: %s\n", NET_INTERFACE, error_content);
        net = 0;
        mask = 0;
        return -1;
    }
    pcap_handle = pcap_open_live(NET_INTERFACE, SNAP_LEN, 1, 0, error_content);
    if (pcap_handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", NET_INTERFACE, error_content);
        return -1;
    }
    if (pcap_compile(pcap_handle, &fp, FILTER_RULE, 0, net) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", FILTER_RULE, error_content);
        return -1;
    }
    if (pcap_setfilter(pcap_handle, &fp)) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", FILTER_RULE, error_content);
        return -1;
    }

    // loop, when packets come, callback function will be called
    pcap_loop(pcap_handle, -1, do_packet, (u_char *)r_conf);

    // clean up
    pcap_freecode(&fp);
    pcap_close(pcap_handle);
    printf("\nCapture complete.\n");
    
    // clean WQ
    exit_report(r_conf);

    return 0;
}
