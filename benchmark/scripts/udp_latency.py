import socket
import time
import os

UDP_IP = "192.168.1.21"
UDP_PORT = 31337

sock = socket.socket(socket.AF_INET,
                             socket.SOCK_DGRAM)
sequence_number = 0
bytes_to_send = os.urandom(1024);
while sequence_number < 20000:
    start = time.time()
    sock.sendto(bytes_to_send, (UDP_IP, UDP_PORT))
    data, _ = sock.recvfrom(1024)
    end = time.time()
    print(end - start)
    sequence_number += 1
