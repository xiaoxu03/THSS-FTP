#include "operator.h"
#include "interpreter.h"

fd_set client_fds;
int max_fd;
int client_status[MAX_CLIENT];
int client_type[MAX_CLIENT];
int client_datafd[MAX_CLIENT];
struct sockaddr_in client_addr[MAX_CLIENT];
int client_cnt = 0;

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

	memset(client_status, DISCONNECTED, sizeof(int) * MAX_CLIENT);
	memset(client_status, ASCII, sizeof(int) * MAX_CLIENT);
	memset(client_addr, 0, sizeof(struct sockaddr_in) * MAX_CLIENT);
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
			else if (eventfd == listenfd){
				// Create a new connection
				int p = 0;
				int connfd;
				if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
					printf("Error accept(): %s(%d)\n", strerror(errno), errno);
					return -1;
				}

				client_status[connfd] = CONNECTED;
				client_datafd[connfd] = 0;
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
				pthread_mutex_lock(&mutex);
				printf("Client %d connected!\n", client_cnt);
				pthread_mutex_unlock(&mutex);
			}
			else{
				ssize_t isize = read(eventfd, in_buf, MAX_BUF);

				if (isize <= 0)
				{
					printf("Disconnected! (fd=%d)\n", eventfd);

					close(eventfd);
					FD_CLR(eventfd, &client_fds);

					client_status[eventfd] = DISCONNECTED;
					client_type[eventfd] = ASCII;
					client_datafd[eventfd] = 0;
					memset(&client_addr[eventfd], 0, sizeof (client_addr[eventfd]));

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
		}
	}
    return 0;
}