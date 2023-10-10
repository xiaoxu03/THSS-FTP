# python3
import socket

size = 8192
sequence = 0

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', 9876))

try:
  while True:
    data, address = sock.recvfrom(size)
    sequence += 1
    echo = str(sequence) + ' ' + data.decode()
    sock.sendto(echo.encode(), address)

finally:
  sock.close()