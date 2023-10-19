#ifndef DATA_H
#define DATA_H


#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>



#define MAX_BUF             8192
#define MAX_CLIENT          1024


// 客户端结构体
typedef struct Client
{
    int status;                     // 客户端的状态
    int control_fd;                 // 客户端的文件描述符
    int data_fd;                    // 客户端的数据连接文件描述符
    int connect_method;             // 客户端的连接方式
    int bytes_type;                 // 客户端的传输字节类型
    int transfer_mode;              // 客户端的数据连接模式
    struct sockaddr_in data_addr;   // 客户端的数据连接地址
    char dir[MAX_BUF];              // 客户端的当前工作目录
} Client;



extern fd_set client_fds;

// 最大的文件描述符
extern int max_fd;

// 用于记录在线客户端的数量
extern int client_cnt;

// 用于记录当前工作目录
extern char dir[MAX_BUF];

// 服务器的输入缓冲区
extern char in_buf[MAX_BUF];

// 服务器的输出缓冲区
extern char out_buf[MAX_BUF];

// 客户端数组
extern Client clients[MAX_CLIENT];



#endif