#include "control.h"
#include "ftputils.h"

int user(char *arg, int client_fd)
{
    if (strcmp(arg + 5, "anonymous"))
    {
        return 1;
    }
    clients[client_fd].status = PASSWORD;
    return 0;
}

int pass(char *arg, int client_fd)
{
    char* str = arg + 5;

    int len = strlen(str);

    // 找到 '@' 和 '.'
    for (int i = 0; i < len; i++) {
        if (str[i] == '@' && i != 0) {
            break;
        }
        if (i == len - 1) {
            return 1;
        }
    }

    clients[client_fd].status = AVAILAVLE;
    return 0;
}

int type(char *arg, int client_fd){
    if(arg[5] == 'A'){
        clients[client_fd].bytes_type = ASCII;
        return ASCII;
    }
    else if(arg[5] == 'I'){
        clients[client_fd].bytes_type = BINARY;
        return BINARY;
    }
    return -1;
}

int port(char *arg, int client_fd){
    int data_socket; // 数据传输的套接字
    struct sockaddr_in data_addr;
    int in_address[4];
    int in_port[2];

    in_port_t _port;
    char ipAddress[INET_ADDRSTRLEN];

    // 解析PORT指令，提取出IP地址和端口号
    if (sscanf(arg, "PORT %d,%d,%d,%d,%d,%d",
        in_address, in_address + 1, in_address + 2, in_address + 3, in_port, in_port + 1) < 6){
            // Invalid ip
            return -1;
        }

    _port = (unsigned short)((*in_port * 256) + *(in_port + 1));
    sprintf(ipAddress, "%d.%d.%d.%d", *in_address, *(in_address + 1), *(in_address + 2), *(in_address + 3));
    if (!(*in_address >= 0 && *in_address <=255) || !(*in_address + 1 >= 0 && *in_address + 1 <=255) || 
        !(*in_address + 2 >= 0 && *in_address + 2 <=255) || !(*in_address + 3 >= 0 && *in_address + 3 <=255) ||
        !(*in_port >= 0 && *in_port <=255) || !(*in_port + 1 >= 0 && *in_port + 1 <=255)){
            return -1;
        }

    // 设置数据传输套接字的地址信息
    memset(&clients[client_fd].data_addr, 0, sizeof(data_addr));
    clients[client_fd].data_addr.sin_family = AF_INET;
    clients[client_fd].data_addr.sin_port = htons(_port);
    inet_pton(AF_INET, ipAddress, &(clients[client_fd].data_addr.sin_addr));

    // 数据传输通道建立成功，可以进行数据传输
    clients[client_fd].transfer_mode = PORT_MODE;
    return 0;
}

int pasv(char *arg, int client_fd){
    if(strlen(arg) > 4){
        return -1;
    }
    
    int data_socket; // 数据传输的套接字
    struct sockaddr_in data_addr;
    socklen_t data_addr_len = sizeof(data_addr);

    // 创建数据传输的套接字
    data_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (data_socket == -1) {
        perror("Error creating data socket");
        return -2;
    }

    // 绑定数据传输的套接字到一个可用端口上
    memset(&data_addr, 0, sizeof(data_addr));
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    data_addr.sin_port = htons(0);

    if (bind(data_socket, (struct sockaddr*)&data_addr, sizeof(data_addr)) == -1) {
        perror("Error binding data socket");
        return -2;
    }

    // 将数据传输套接字存储起来
    clients[client_fd].data_fd = data_socket;

    // 将数据传输套接字设置为监听模式
    if (listen(clients[client_fd].data_fd, 1) == -1) {
        perror("Error listening on data socket");
        return -2;
    }

    data_addr_len = sizeof(data_addr);
    if (getsockname(data_socket, (struct sockaddr*)&data_addr, &data_addr_len) == -1) {
        perror("Error getting data socket name");
        return -2;
    }
    int port = ntohs(data_addr.sin_port);

    // Change IP format
    char* ip_address = inet_ntoa(data_addr.sin_addr);
    char* temp = ip_address;
    while (*temp){
        if (*temp == '.'){
            *temp = ',';
        }
        temp ++;
    }
    clients[client_fd].transfer_mode = PASV_MODE;
    in_port_t _port = ntohs(data_addr.sin_port);
    snprintf(out_buf, sizeof(out_buf), "227 Entering Passive Mode (%s,%u,%u).\r\n",
        ip_address, _port >> 8, _port & 0xFF);

    printf("%s\n", out_buf);

    return 0;
}

int retr(char *arg, int client_fd){
    if(strlen(arg) < 6){
        return -1;
    }

    char * filename = arg + 5;

    FILE* file;
    char buffer[MAX_BUF];
    ssize_t bytes_read;

    if(clients[client_fd].transfer_mode == PORT_MODE){
        // 创建数据传输的套接字
        int data_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (data_fd == -1) {
            perror("Error creating data socket");
            return -1;
        }

        // 连接到客户端指定的IP地址和端口号
        if (connect(data_fd, (struct sockaddr*)&clients[client_fd].data_addr, sizeof(clients[client_fd].data_addr)) == -1) {
            perror("Error connecting to client");
            return -3;
        }

        clients[client_fd].data_fd = data_fd;
    }
    else if(clients[client_fd].transfer_mode == PASV_MODE){
        // PASV accept
        int pasv_datafd = accept(clients[client_fd].data_fd, NULL, NULL);
        if(pasv_datafd == -1){
            perror("Error connecting to client");
            return -4;
        }
        close(clients[client_fd].data_fd);
        clients[client_fd].data_fd = pasv_datafd;
    }
    else{
        return -5;
    }

    // 接收到RETR指令后，解析出要下载的文件路径
    char filedir[MAX_BUF];
    char realdir[MAX_BUF];

    connect_dir(clients[client_fd].dir, filename, filedir);
    real_dir(filedir, realdir);

    // 打开要下载的文件
    file = fopen(realdir, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return -2;
    }
    char mode[10];

    if(clients[client_fd].bytes_type == BINARY){
        strcpy(mode, "BINARY");
    }
    else{
        strcpy(mode, "ASCII");
    }

    fseek(file, 0, SEEK_END);

    char message[MAX_BUF];
    char size[1024];
    sprintf(size, "%ld", ftell(file));
    rewind(file);
    sprintf(message, "150 Opening %s mode data connection for %s(%s bytes).\r\n", mode, filename, size);
    send_msg(message, client_fd, -1);

    // 使用数据连接向客户端发送文件数据
    while ((bytes_read = fread(buffer, 1, MAX_BUF, file)) > 0) {
        if (send(clients[client_fd].data_fd, buffer, bytes_read, 0) == -1) {
            perror("Error sending file data");
            return -2;
        }
    }

    fclose(file);
    close(clients[client_fd].data_fd);
    clients[client_fd].data_fd = 0;
    clients[client_fd].transfer_mode = NONE_MODE;

    return 0;
}

int stor(char *arg, int client_fd){
    if(strlen(arg) < 6){
        return -1;
    }

    char * filename = arg + 5;

    FILE* file;
    char buffer[MAX_BUF];
    ssize_t bytes_read;

    if(clients[client_fd].transfer_mode == PORT_MODE){
        // 创建数据传输的套接字
        int data_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (data_fd == -1) {
            perror("Error creating data socket");
            return -1;
        }

        // 连接到客户端指定的IP地址和端口号
        if (connect(data_fd, (struct sockaddr*)&clients[client_fd].data_addr, sizeof(clients[client_fd].data_addr)) == -1) {
            perror("Error connecting to client");
            return -3;
        }

        clients[client_fd].data_fd = data_fd;
    }
    else if(clients[client_fd].transfer_mode == PASV_MODE){
        // PASV accept
        int pasv_datafd = accept(clients[client_fd].data_fd, NULL, NULL);
        if(pasv_datafd == -1){
            perror("Error connecting to client");
            return -4;
        }
        close(clients[client_fd].data_fd);
        clients[client_fd].data_fd = pasv_datafd;
    }
    else{
        return -5;
    }

    // 接收到RETR指令后，解析出要下载的文件路径
    char filedir[MAX_BUF];
    char realdir[MAX_BUF];

    connect_dir(clients[client_fd].dir, filename, filedir);
    real_dir(filedir, realdir);

    // 创建要上传的文件
    file = fopen(realdir, "w+");
    if (file == NULL) {
        perror("Error opening file");
        return -2;
    }
    char mode[10];

    if(clients[client_fd].bytes_type == BINARY){
        strcpy(mode, "BINARY");
    }
    else{
        strcpy(mode, "ASCII");
    }

    fseek(file, 0, SEEK_END);

    char message[MAX_BUF];
    char size[1024];
    sprintf(size, "%ld", ftell(file));
    sprintf(message, "150 Opening %s mode data connection for %s.\r\n", mode, filename);
    send_msg(message, client_fd, -1);


    int recv_flag;
    // 使用数据连接向客户端发送文件数据
    while((recv_flag = recv(clients[client_fd].data_fd, buffer, MAX_BUF, 0)) > 0){
        int bytes_write = fwrite(buffer, sizeof(char), recv_flag, file);
    }

    if(recv_flag == -1){
        return -3;
    }

    fclose(file);
    close(clients[client_fd].data_fd);
    clients[client_fd].data_fd = 0;
    clients[client_fd].transfer_mode = NONE_MODE;

    return 0;
}

int quit(char *arg, int client_fd){
    if(strlen(arg) > 4){
        return -1;
    }
    return 0;
}

int cwd(char *arg, int client_fd){
    if(strlen(arg) < 4){
        return -1;
    }

    char * dirname = arg + 4;
    DIR *opened_dir;
    struct dirent *ptr;

    char newdir[MAX_BUF];
    char realdir[MAX_BUF];

    int opt = connect_dir(clients[client_fd].dir, dirname, newdir);
    real_dir(newdir, realdir);

    if (opt == -1) {
        // 路径过长
        return -1;
    }

    
    if((opened_dir = opendir(realdir)) == NULL){
        // 路径不存在
        return -2;
    }

    
    strcpy(clients[client_fd].dir, newdir);
    closedir(opened_dir);

    return 0;
}


int mkd(char *arg, int client_fd){
    if(strlen(arg) < 4){
        return -1;
    }

    char * dirname = arg + 4;
    char newdir[MAX_BUF];
    char realdir[MAX_BUF];

    if (dirname[0] == '/')
    {
        real_dir(dirname, realdir);
    }
    else
    {
        connect_dir(clients[client_fd].dir, dirname, newdir);
        real_dir(newdir, realdir);
    }

    printf("make dir: %s\n", realdir);

    // 创建目录
    if (mkdir(realdir, 0777) == -1) {
        // 创建目录失败
        return -2;
    }
    return 0;
}

int list(char *arg, int client_fd){
    char message[MAX_BUF];
    sprintf(message, "150 Opening ASCII mode data connection for file list.\r\n");
    send_msg(message, client_fd, -1);

    int max_size_len = 0;
    // 遍历目录
    struct dirent *ptr;
    DIR *opened_dir;
    char realdir[MAX_BUF];
    real_dir(clients[client_fd].dir, realdir);
    if((opened_dir = opendir(realdir)) == NULL){
        // 路径不存在
        return -1;
    }
    while(ptr = readdir(opened_dir)){
        // 忽略 . 和 ..
        if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0 ){
            continue;
        }

        // 获取文件信息
        char file_dir[1024];
        file_dir[0] = '\0';
        strcat(file_dir, realdir);
        if (realdir[strlen(realdir) - 1] != '/') {
            strcat(file_dir, "/");
        }
        strcat(file_dir, ptr->d_name);
        struct stat file_stat;
        if (stat(file_dir, &file_stat) == -1) {
            printf("Load failed!\n");
            continue;
        }

        int file_size = file_stat.st_size;
        if(file_size > max_size_len){
            max_size_len = file_size;
        }
    }
    char size[1024];
    max_size_len = snprintf(size, sizeof(size), "%d", max_size_len);
    closedir(opened_dir);

    // 打开当前目录
    if((opened_dir = opendir(realdir)) == NULL){
        // 路径不存在
        return -1;
    }
    printf("real dir: %s\n", realdir);
    // 使用数据连接向客户端发送文件数据
    if(clients[client_fd].transfer_mode == PORT_MODE){
        // 创建数据传输的套接字
        int data_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (data_fd == -1) {
            perror("Error creating data socket");
            return -1;
        }
        // 连接到客户端指定的IP地址和端口号
        if (connect(data_fd, (struct sockaddr*)&clients[client_fd].data_addr, sizeof(clients[client_fd].data_addr)) == -1) {
            perror("Error connecting to client");
            return -3;
        }

        clients[client_fd].data_fd = data_fd;
    }
    else if(clients[client_fd].transfer_mode == PASV_MODE){
        // PASV accept
        int pasv_datafd = accept(clients[client_fd].data_fd, NULL, NULL);
        if(pasv_datafd == -1){
            perror("Error connecting to client");
            return -4;
        }
        close(clients[client_fd].data_fd);
        clients[client_fd].data_fd = pasv_datafd;
    }
    else{
        return -5;
    }

    // 遍历目录
    while((ptr = readdir(opened_dir)) != NULL){
        // 忽略 . 和 ..
        if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0 ){
            continue;
        }
    
        // 获取文件信息
        char file_info[256];
        char file_dir[256];
        file_dir[0] = '\0';
        strcat(file_dir, realdir);
        strcat(file_dir, ptr->d_name);
        
    
        if (!format_file_info(file_info, file_dir, max_size_len)) {
            printf("Load failed!\n");
            continue;
        }
        strcat(file_info, " ");
        strcat(file_info, ptr->d_name);
        strcat(file_info, "\r\n");
        send_msg(file_info, clients[client_fd].data_fd, -1);
    }

    // 关闭目录
    closedir(opened_dir);

    // 关闭数据连接
    close(clients[client_fd].data_fd);
    clients[client_fd].data_fd = 0;
    clients[client_fd].transfer_mode = NONE_MODE;

    // 发送完成消息
    sprintf(message, "226 Transfer complete.\r\n");
    send_msg(message, client_fd, -1);

    return 0;
}

int rmd(char *arg, int client_fd){
    if(strlen(arg) < 4){
        return -1;
    }

    char * dirname = arg + 4;
    char newdir[MAX_BUF];

    int opt = connect_dir(clients[client_fd].dir, dirname, newdir);

    if (opt == -1) {
        // 路径过长
        return -1;
    }

    // 删除目录
    if (rmdir(newdir) == -1) {
        // 删除目录失败
        return -2;
    }

    return 0;
}

int rnfr(char *arg, int client_fd){
    if(strlen(arg) < 5){
        return -1;
    }

    char * filename = arg + 5;
    char newdir[MAX_BUF];
    char realdir[MAX_BUF];

    int opt = connect_dir(clients[client_fd].dir, filename, newdir);
    real_dir(newdir, realdir);

    if (opt == -1) {
        // 路径过长
        return -1;
    }

    // 检查文件是否存在
    if (access(realdir, F_OK) == -1) {
        // 文件不存在
        return -2;
    }

    strcpy(clients[client_fd].rndir, realdir);

    return 0;
}

int rnto(char *arg, int client_fd){
    if(strlen(arg) < 5){
        return -1;
    }

    char * filename = arg + 5;
    char newdir[MAX_BUF];
    char realdir[MAX_BUF];

    int opt = connect_dir(clients[client_fd].dir, filename, newdir);
    real_dir(newdir, realdir);

    if (opt == -1) {
        // 路径过长
        return -1;
    }

    // 重命名文件
    if (rename(clients[client_fd].rndir, realdir) == -1) {
        // 重命名失败
        return -2;
    }

    return 0;
}