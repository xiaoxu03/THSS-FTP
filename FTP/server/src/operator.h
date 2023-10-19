#ifndef OPREATOR_H
#define OPERATOR_H


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

#include "status.h"
#include "data.h"


int control_init(in_port_t _port);


#endif