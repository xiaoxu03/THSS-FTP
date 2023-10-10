#ifndef INTERPRETER_H
#define INTERPRETER_H


#include <string.h>
#include <sys/select.h>
#include <stdio.h>

#include "status.h"
#include "data.h"
#include "control.h"


// Macros of commands
#define ERROR   -1
#define USER    1
#define PASS    2
#define PORT    3
#define PASV    4
#define RETR    5
#define STOR    6
#define SYST    7
#define TYPE    8
#define QUIT    9
#define MKD     10
#define CWD     11
#define PWD     12
#define LIST    13
#define RMD     14
#define RNFR    15
#define RNTO    16


int interpret_comand(char *_command);

int interpret(int client_fd);

#endif