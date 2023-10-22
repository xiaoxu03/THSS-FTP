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

#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>

#include "data.h"


#define AUTO -1


int send_msg(char *_msg, int _dest, int _len);

int connect_dir(char *_father, char *_son, char *_dest);

char* format_file_info(char* buffer, const char* filename, int max_size_len);

int real_dir(char *_path, char *_dest);

int is_root(char *_path);

#endif