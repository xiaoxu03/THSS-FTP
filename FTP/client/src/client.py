import socket

# 定义服务器地址和端口
server_address = ('127.0.0.1', 21)

# 创建套接字并连接到服务器
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
sock.connect(server_address)

# 接收响应
response = sock.recv(1024)  # 一次最多接收1024字节的数据
print("SERVER: " + response.decode())  # 打印响应消息（将字节串解码为字符串）

i = 0
while(i < 3):
    # 发送消息
    message = input("CLIENT: ").encode()  # 将消息转换为字节串
    sock.sendall(message)
    i += 1


# 关闭套接字连接
sock.close()