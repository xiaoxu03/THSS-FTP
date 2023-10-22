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
    // 否则就返回父路径+子路径
    else {
        strcpy(_dest, _father);
        strcat(_dest, "/");
        strcat(_dest, _son);
    }

    // 处理路径中的特殊情况
    char *p = _dest;
    while (*p != '\0') {
        if (strncmp(p, "//", 2) == 0) {
            memmove(p, p + 1, strlen(p + 1) + 1);
        } else if (strncmp(p, "/./", 3) == 0 || strncmp(p, "/.\0", 3) == 0) {
            memmove(p, p + 2, strlen(p + 2) + 1);
        } else if (strncmp(p, "/../", 4) == 0 || strncmp(p, "/..\0", 4) == 0) {
            // 如果新目录在根目录之上，就返回-1
            if (p == _dest || strncmp(p - 1, "/", 1) == 0) {
                return -1;
            }
            // 否则，找到前一个'/'的位置
            *p = '\0';
            char *prev_slash = strrchr(_dest, '/');
            if (prev_slash == NULL) {
                return -1;
            }
            // 移除前一个'/'和'/'之间的内容
            memmove(prev_slash + 1, p + 3, strlen(p + 3) + 1);
            p = prev_slash;
        } else {
            p++;
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