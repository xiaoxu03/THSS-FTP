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

int connect_dir(char *_father, char *_son, char *_dest){
    if (strlen(_father) + strlen(_son) + 2 > MAX_BUF) {
        return -1;
    }

    // 如果以'/'开始，就直接返回子路径
    if (_son[0] == '/') {
        strcpy(_dest, _son);
    }
    // 如果以'/'结尾，就直接返回父路径
    else if (_father[strlen(_father) - 1] == '/') {
        strcpy(_dest, _father);
        strcat(_dest, _son);
    }
    // 否则就返回父路径+子路径
    else {
        strcpy(_dest, _father);
        strcat(_dest, "/");
        strcat(_dest, _son);
    }

    //处理一下，消去_dest路径中的".."和".", 以及"//", 得到最终的路径
    int len = strlen(_dest);
    int i = 0;

    while (i < len)
    {
        // 如果有"//", 就消去
        if(_dest[i] == '/' && _dest[i + 1] == '/'){
            int j = i + 1;
            while (j < len) {
                _dest[j] = _dest[j + 1];
                j++;
            }
            len--;
        }
        // 如果有"/./", 就消去
        else if(_dest[i] == '/' && _dest[i + 1] == '.' && _dest[i + 2] == '/'){
            int j = i + 1;
            while (j < len) {
                _dest[j] = _dest[j + 2];
                j++;
            }
            len -= 2;
        }
        // 如果有"/../", 就消去
        else if(_dest[i] == '/' && _dest[i + 1] == '.' && _dest[i + 2] == '.' && _dest[i + 3] == '/'){
            int j = i + 1;
            while (j < len) {
                _dest[j] = _dest[j + 3];
                j++;
            }
            len -= 3;
            // 如果消去后，_dest[i]前面有'/', 就把这个'/'也消去
            if(i > 0 && _dest[i - 1] == '/'){
                j = i - 1;
                while (j < len) {
                    _dest[j] = _dest[j + 1];
                    j++;
                }
                len--;
            }
            // 如果消去后，_dest[i]前面没有'/', 就把这个'/'也消去
            else if(i == 0){
                j = i;
                while (j < len) {
                    _dest[j] = _dest[j + 1];
                    j++;
                }
                len--;
            }
        }
        else{
            i++;
        }
    }
    

    return 0;
}
