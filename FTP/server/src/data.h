#ifndef DATA_H
#define DATA_H


#include <sys/select.h>


#define MAX_BUF             8192
#define MAX_CLIENT          1024


extern fd_set client_fds;

// 最大的文件描述符
extern int max_fd;

// 用于记录客户端的连接状态
extern int client_status[MAX_CLIENT];

// 用于记录客户端的发送字节类型
extern int client_type[MAX_CLIENT];

// 用于记录客户端的数据连接文件描述符
extern int client_datafd[MAX_CLIENT];

// 用于记录客户端的数据主动被动模式
extern int client_mode[MAX_CLIENT];

// 用于记录在线客户端的数量
extern int client_cnt;

// 用于记录当前工作目录
extern char dir[MAX_BUF];

// 服务器的输入缓冲区
extern char in_buf[MAX_BUF];

// 服务器的输出缓冲区
extern char out_buf[MAX_BUF];

// 用于记录客户端数据传输连接的地址
extern struct sockaddr_in client_addr[MAX_CLIENT];

#endif