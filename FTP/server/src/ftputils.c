#include "ftputils.h"

int send_msg(char *_msg, int _dest, int _len){
    int p = 0;
    if(_len == AUTO){
        _len = strlen(_msg);
    }
    while (p < _len) {
        int n = write(_dest, _msg + p, _len - p);
        if (n < 0) {
            printf("Error write(): %s(%d)\n", strerror(errno), errno);
            return -1;
        } else {
            p += n;
        }			
    }
    return 0;
}

