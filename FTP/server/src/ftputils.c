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

int real_dir(char *_path, char *_dest){
    char temp[MAX_BUF];
    strcpy(temp, dir);
    if( temp[strlen(temp) - 1] != '/'){
        strcat(temp, "/");
    }
    strcat(temp, _path);
    strcpy(_dest, temp);
    return 0;
}

// 根据 uid 获取用户名
char* get_username(uid_t uid) {
    struct passwd* pwd = getpwuid(uid);
    if (pwd == NULL) {
        return NULL;
    }
    return pwd->pw_name;
}

// 将文件信息转换为字符串
char* format_file_info(char* buffer, const char* filename, int max_size_len) {
    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) {
        return NULL;
    }
    int type = (S_ISDIR(file_stat.st_mode)) ? 2 : 1;
    // 获取文件权限
    char mode[11];
    mode_t file_mode = file_stat.st_mode;
    mode[0] = (S_ISDIR(file_mode)) ? 'd' : '-';
    mode[1] = (file_mode & S_IRUSR) ? 'r' : '-';
    mode[2] = (file_mode & S_IWUSR) ? 'w' : '-';
    mode[3] = (file_mode & S_IXUSR) ? 'x' : '-';
    mode[4] = (file_mode & S_IRGRP) ? 'r' : '-';
    mode[5] = (file_mode & S_IWGRP) ? 'w' : '-';
    mode[6] = (file_mode & S_IXGRP) ? 'x' : '-';
    mode[7] = (file_mode & S_IROTH) ? 'r' : '-';
    mode[8] = (file_mode & S_IWOTH) ? 'w' : '-';
    mode[9] = (file_mode & S_IXOTH) ? 'x' : '-';
    mode[10] = '\0';

    // 获取文件所有者和大小
    char owner[32];
    snprintf(owner, 32, "%s", get_username(file_stat.st_uid));
    char size[32];
    off_t file_size = file_stat.st_size;
    sprintf(size, "%ld", file_size);

    // 获取文件修改时间
    char time_str[32];
    time_t file_time = file_stat.st_mtime;
    struct tm* time_info = localtime(&file_time);
    strftime(time_str, 32, "%b %d %H:%M", time_info);
    char table[128];
    memset(table, ' ', 128);
    table[max_size_len - (int)strlen(size)] = '\0';
    strcat(table, size);
    // 将文件信息格式化为字符串
    snprintf(buffer, 256, "%s %d %s %s %s %s", mode, type, owner, owner, table, time_str);
    return buffer;
}