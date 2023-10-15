#ifndef UTILS_H
#define UTILS_H


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "data.h"


#define AUTO -1


int send_msg(char *_msg, int _dest, int _len);

int connect_dir(char *_father, char *_son, char *_dest);

#endif