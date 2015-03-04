#ifndef _REPORT_H
#define _REPORT_H

extern int get_mq_conf();
extern int get_mqserver_hostbyname();

extern int ready_to_send();
extern int send_msgs_to_mq();

extern int init_msgs_buf();
extern void distory_msgs_buf();

extern int init_mqsock();
extern void close_mqsock();

extern int add_msg(unsigned long time, unsigned int sip, unsigned int dip, char *mmac, char *gmac, char *data, int data_len);

#endif
