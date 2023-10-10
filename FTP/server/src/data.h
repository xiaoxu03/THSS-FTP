#ifndef DATA_H
#define DATA_H


#include <sys/select.h>


#define MAX_BUF             8192
#define MAX_CLIENT          1024


extern fd_set client_fds;
extern int max_fd;
extern int client_status[MAX_CLIENT];
extern int client_type[MAX_CLIENT];
extern int client_datafd[MAX_CLIENT];
extern int client_cnt;
extern char dir[MAX_BUF];
extern char in_buf[MAX_BUF];
extern char out_buf[MAX_BUF];
extern struct sockaddr_in client_addr[MAX_CLIENT];

#endif