#include <stdio.h>
#include <stdlib.h>

#include "interpreter.h"
#include "operator.h"

char dir[MAX_BUF];
char in_buf[MAX_BUF];
char out_buf[MAX_BUF];
in_port_t server_port;

int main(int argc, char** argv){
    
    int hp = 0;
    int hd = 0;

    for(int i = 1;i < argc;i++){
        if(!strcmp(argv[i], "-port")){
            if(i + 1 < argc){
                server_port = atoi(argv[i + 1]);
                if(port){
                    i++;
                    hp = 1;
                }
            }
        }
    }

    for(int i = 1;i < argc;i++){
        if(!strcmp(argv[i], "-root")){
            if(i + 1 < argc){
                strcpy(dir, argv[i + 1]);
                i++;
                hd = 1;
            }
        }
    }

    if(!hp) {
        server_port = 21;
    }

    if(!hd) {
        strcpy(dir, "/tmp");
    }

    if(control_init(server_port)){
        printf("Failed to launch!\n");
        return -1;
    }

    return 0;
}