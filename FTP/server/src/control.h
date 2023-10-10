#ifndef CONTROL_H
#define CONTROL_H


#include <string.h>
#include <regex.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>

#include "data.h"
#include "status.h"


int user(char *arg, int client_fd);

int pass(char *arg, int client_fd);

int type(char *arg, int client_fd);

int port(char *arg, int client_fd);

int pasv(char *arg, int client_fd);

int retr(char *arg, int client_fd);

int stor(char *arg, int client_fd);

int quit(char *arg, int client_fd);

#endif