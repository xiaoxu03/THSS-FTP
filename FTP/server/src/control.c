#include "control.h"
#include "ftputils.h"

int user(char *arg, int client_fd)
{
    if (strcmp(arg + 5, "anonymous"))
    {
        return 1;
    }
    client_status[client_fd] = PASSWORD;
    return 0;
}

int pass(char *arg, int client_fd)
{
    int status = 0, i;
    int cflags = REG_EXTENDED;
    regmatch_t pmatch[1];
    const size_t nmatch = 1;
    regex_t reg;
    const char *pattern = "^\\w+([-+.]\\w+)*@([-.]\\w+)*$";
    regcomp(&reg, pattern, cflags);         // 编译正则模式
    status = regexec(&reg, arg + 5, nmatch, pmatch, 0); // 执行正则表达式和缓存的比较

    regfree(&reg);

    if (status == REG_NOMATCH)
    {
        return 1;
    }

    client_status[client_fd] = AVAILAVLE;
    return 0;
}

int type(char *arg, int client_fd){
    if(arg[5] == 'A'){
        client_type[client_fd] = ASCII;
        return ASCII;
    }
    else if(arg[5] == 'I'){
        client_type[client_fd] = BINARY;
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
    memset(&client_addr[client_fd], 0, sizeof(data_addr));
    client_addr[client_fd].sin_family = AF_INET;
    client_addr[client_fd].sin_port = htons(_port);
    inet_pton(AF_INET, ipAddress, &(client_addr[client_fd].sin_addr));

    // 数据传输通道建立成功，可以进行数据传输
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

    if (bind(data_socket, (struct sockaddr*)&data_addr, sizeof(data_addr)) == -1) {
        perror("Error binding data socket");
        return -2;
    }

    // 将数据传输套接字设置为监听模式
    if (listen(data_socket, 1) == -1) {
        perror("Error listening on data socket");
        return -2;
    }

    // 获取数据传输套接字绑定的端口号
    if (getsockname(data_socket, (struct sockaddr*)&data_addr, &data_addr_len) == -1) {
        perror("Error getting data socket address");
        return -2;
    }    

    // Change IP format
    char* ip_address = inet_ntoa(data_addr.sin_addr);
    char* temp = ip_address;
    while (*temp){
        if (*temp == '.'){
            *temp = ',';
        }
        temp ++;
    }
    
    in_port_t _port = ntohs(data_addr.sin_port);
    snprintf(out_buf, sizeof(out_buf), "227 Entering Passive Mode (%s,%u,%u).\r\n",
        ip_address, _port >> 8, _port & 0xFF);

    client_datafd[client_fd] = data_socket;

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

    int data_fd;

    if(client_addr[client_fd].sin_addr.s_addr || client_addr[client_fd].sin_port){
        // 创建数据传输的套接字
        data_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (data_fd == -1) {
            perror("Error creating data socket");
            return -1;
        }

        // 连接到客户端指定的IP地址和端口号
        if (connect(data_fd, (struct sockaddr*)&client_addr[client_fd], sizeof(client_addr[client_fd])) == -1) {
            perror("Error connecting to client");
            return -3;
        }

        client_datafd[client_fd] = data_fd;
        memset(&client_addr[client_fd], 0, sizeof(client_addr[client_fd]));
    }
    else if(client_datafd[client_fd]){
        // PASV accept
        int pasv_datafd = accept(client_datafd[client_fd], NULL, NULL);
        if(pasv_datafd == -1){
            perror("Error connecting to client");
            return -4;
        }
        
        client_datafd[client_fd] = pasv_datafd;
    }

    // 接收到RETR指令后，解析出要下载的文件路径
    char filedir[MAX_BUF];

    strcpy(filedir, dir);
    strcat(filedir, "/");
    strcat(filedir, filename);

    // 打开要下载的文件
    file = fopen(filedir, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return -2;
    }
    char mode[10];

    if(client_type[client_fd] == BINARY){
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
        if (send(client_datafd[client_fd], buffer, bytes_read, 0) == -1) {
            perror("Error sending file data");
            return -2;
        }
    }

    fclose(file);
    close(client_datafd[client_fd]);
    client_datafd[client_fd] = 0;
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

    int data_fd;

    if(client_addr[client_fd].sin_addr.s_addr || client_addr[client_fd].sin_port){
        // 创建数据传输的套接字
        data_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (data_fd == -1) {
            perror("Error creating data socket");
            return -1;
        }

        // 连接到客户端指定的IP地址和端口号
        if (connect(data_fd, (struct sockaddr*)&client_addr[client_fd], sizeof(client_addr[client_fd])) == -1) {
            perror("Error connecting to client");
            return -3;
        }

        client_datafd[client_fd] = data_fd;
        memset(&client_addr[client_fd], 0, sizeof(client_addr[client_fd]));
    }
    else if(client_datafd[client_fd]){
        // PASV accept
        int pasv_datafd = accept(client_datafd[client_fd], NULL, NULL);
        if(pasv_datafd == -1){
            perror("Error connecting to client");
            return -4;
        }
        
        client_datafd[client_fd] = pasv_datafd;
    }

    // 接收到RETR指令后，解析出要下载的文件路径
    char filedir[MAX_BUF];

    strcpy(filedir, dir);
    strcat(filedir, "/");
    strcat(filedir, filename);

    // 创建要上传的文件
    file = fopen(filedir, "w+");
    if (file == NULL) {
        perror("Error opening file");
        return -2;
    }
    char mode[10];

    if(client_type[client_fd] == BINARY){
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
    while((recv_flag = recv(client_datafd[client_fd], buffer, MAX_BUF, 0)) > 0){
        int bytes_write = fwrite(buffer, sizeof(char), recv_flag, file);
    }

    if(recv_flag == -1){
        return -3;
    }

    fclose(file);
    close(client_datafd[client_fd]);
    client_datafd[client_fd] = 0;
    return 0;
}

int quit(char *arg, int client_fd){
    if(strlen(arg) != 4){
        return -1;
    }

    FD_CLR(client_fd, &client_fds);
    

    client_status[client_fd] = DISCONNECTED;
    client_type[client_fd] = ASCII;
    client_datafd[client_fd] = 0;
    memset(&client_addr[client_fd], 0, sizeof (client_addr[client_fd]));

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

    int opt = connect_dir(dir, dirname, newdir);

    if (opt == -1) {
        // 路径过长
        return -1;
    }

    
    if((opened_dir = opendir(newdir)) == NULL){
        // 路径不存在
        return -2;
    }

    
    strcpy(dir, newdir);
    closedir(opened_dir);

    return 0;
}


int mkd(char *arg, int client_fd){
    if(strlen(arg) < 4){
        return -1;
    }

    char * dirname = arg + 4;
    char newdir[MAX_BUF];

    int opt = connect_dir(dir, dirname, newdir);

    if (opt == -1) {
        // 路径过长
        return -1;
    }

    // 创建新目录
    if (mkdir(newdir, 0777) == -1) {
        // 创建目录失败
        return -2;
    }

    return 0;
}

int list(char *arg, int client_fd){
    char message[MAX_BUF];
    sprintf(message, "150 Opening ASCII mode data connection for file list.\r\n");
    send_msg(message, client_fd, -1);

    // 打开当前目录
    DIR *opened_dir;
    if((opened_dir = opendir(dir)) == NULL){
        // 路径不存在
        return -1;
    }

    // 遍历目录
    struct dirent *ptr;
    while((ptr = readdir(opened_dir)) != NULL){
        // 忽略 . 和 ..
        if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0){
            continue;
        }

        // 发送文件名
        sprintf(message, "%s\r\n", ptr->d_name);
        send_msg(message, client_datafd[client_fd], -1);
    }

    // 关闭目录
    closedir(opened_dir);

    // 关闭数据连接
    close(client_datafd[client_fd]);
    client_datafd[client_fd] = 0;

    // 发送完成消息
    sprintf(message, "226 Transfer complete.\r\n");
    send_msg(message, client_fd, -1);

    return 0;
}