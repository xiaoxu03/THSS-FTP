import socket

HOST = 'ftp.server.com'
PORT = 21

# Connect to the FTP server
ftp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
ftp_socket.connect((HOST, PORT))

# Receive the welcome message from the server
response = ftp_socket.recv(1024)
print(response.decode())

# Send the username and password
ftp_socket.sendall(b'USER anonymous\r\n')
response = ftp_socket.recv(1024)
print(response.decode())

ftp_socket.sendall(b'PASS chao@chao.chao\r\n')
response = ftp_socket.recv(1024)
print(response.decode())

# Change to the desired directory
ftp_socket.sendall(b'CWD /path/to/directory\r\n')
response = ftp_socket.recv(1024)
print(response.decode())

# Download a file
ftp_socket.sendall(b'RETR filename.txt\r\n')
response = ftp_socket.recv(1024)
print(response.decode())

with open('filename.txt', 'wb') as f:
    while True:
        data = ftp_socket.recv(1024)
        if not data:
            break
        f.write(data)

# Close the connection
ftp_socket.sendall(b'QUIT\r\n')
response = ftp_socket.recv(1024)
print(response.decode())
ftp_socket.close()
