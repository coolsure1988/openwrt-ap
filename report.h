#ifndef _REPORT_H
#define _REPORT_H

typedef struct report_conf{
	char mqserver[64];
	int mqport;

	int mqsockfd;
	struct in_addr server_addr;

	char mmac[18];

	FILE *stat;
        unsigned long stat_count;
}report_conf_s;

typedef struct report_buf{
	char *msgs;
        int msgs_len;
        int count;
	int max_count;
}report_buf_s;

extern report_conf_s *init_report();
extern void exit_report(report_conf_s *r_conf);

extern int ready_to_send();
extern int send_msgs_to_mq(report_conf_s *r_conf);

extern int init_mqsock(report_conf_s *r_conf);
extern void close_mqsock(report_conf_s *r_conf);

extern int add_msg(unsigned long time, unsigned int sip, unsigned int dip, char *mmac, char *gmac, char *data, int data_len);

#endif
