#include "operator.h"
#include "interpreter.h"

fd_set client_fds;
int max_fd;
int client_cnt = 0;
Client clients[MAX_CLIENT];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int control_init(in_port_t _port){
    pthread_t listenid;
    int listenfd;
    struct sockaddr_in addr;

	// 创建socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	// 设置本机的ip和port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"0.0.0.0"

	// 将本机的ip和port与socket绑定
	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

    // 开始监听socket
	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	printf("FTP server started on port: %u!\n", ntohs(addr.sin_port));
	printf("FTP server directory set: %s!\n", dir);

	// Deal with control messages
	char welcome_sentence[] = "220 XCs FTP server ready.\r\n";
    int len = strlen(welcome_sentence);

	memset(clients, 0, sizeof(clients));
	FD_ZERO(&client_fds);
	
    FD_SET(listenfd, &client_fds);
	max_fd = listenfd;

	while(1) {
		// Copy fd set of clients
		fd_set temp_fds = client_fds;

		int infds = select(max_fd + 1, &temp_fds, NULL, NULL, NULL);

		if(infds < 0){
			printf("Error select(): %s(%d)\n", strerror(errno), errno);
			return 1;
		}
		else if(infds == 0){
			printf("Select time out!\n");
			continue;
		}

		for (int eventfd = 0; eventfd < max_fd + 1; eventfd++){
			if (FD_ISSET(eventfd, &temp_fds) <= 0){
				continue;
			}
			// 连接请求
			else if (eventfd == listenfd){
				// Create a new connection
				int p = 0;
				int connfd;
				if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
					printf("Error accept(): %s(%d)\n", strerror(errno), errno);
					return -1;
				}

				clients[connfd].status = CONNECTED;
				FD_SET(connfd, &client_fds);
				if (connfd > max_fd) {
					max_fd = connfd;
				}

				//发送字符串到socket
				p = 0;
				while (p < len) {
					int n = write(connfd, welcome_sentence + p, len - p);
					if (n < 0) {
						printf("Error write(): %s(%d)\n", strerror(errno), errno);
						return -1;
					} else {
						p += n;
					}			
				}

				client_cnt ++;
			}
			// 客户端请求
			else if(clients[eventfd].status){
				ssize_t isize = read(eventfd, in_buf, MAX_BUF);

				if (isize <= 0)
				{
					printf("Disconnected! (fd=%d)\n", eventfd);

					close(eventfd);
					FD_CLR(eventfd, &client_fds);

					memset(&clients[eventfd], 0, sizeof(clients[eventfd]));

					// Refind the largest fd in fd set
					if (eventfd == max_fd){
						for(int i = max_fd; i > 0; i--){
							if (FD_ISSET(i, &client_fds)){
								max_fd = i;
								break;
							}
						}
					}

					continue;
				}
				else{
					int len = 0;
					len += isize;
					while(!in_buf[len - 1]){
						len += read(eventfd, in_buf + len, MAX_BUF - len);
					}
					in_buf[len - 2] = 0;
					printf("%s\n", in_buf);
					interpret(eventfd);
				}
			}
			// 文件传输请求
			else{
				//TODO: 文件传输线程监听
			}
		}
	}
    return 0;
}