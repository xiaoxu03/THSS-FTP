#include <stdio.h>
#include <stdlib.h>

#include "interpreter.h"
#include "operator.h"

char dir[MAX_BUF];
char in_buf[MAX_BUF];
char out_buf[MAX_BUF];


int main(int argc, char** argv){
    in_port_t port;
    int hp = 0;
    int hd = 0;

    for(int i = 1;i < argc;i++){
        if(!strcmp(argv[i], "-port")){
            if(i + 1 < argc){
                port = atoi(argv[i + 1]);
                if(port){
                    i++;
                    hp = 1;
                }
            }
        }
    }

    for(int i = 1;i < argc;i++){
        if(!strcmp(argv[i], "-dir")){
            if(i + 1 < argc){
                strcpy(dir, argv[i + 1]);
                i++;
                hd = 1;
            }
        }
    }

    if(!hp) {
        printf("Please input port with \"-port\" argument!\n");
        return -1;
    }

    if(!hd) {
        printf("Please input port with \"-dir\" argument!\n");
        return -1;
    }

    if(control_init(port)){
        printf("Failed to launch!\n");
        return -1;
    }

    return 0;
}