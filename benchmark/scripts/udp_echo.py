import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = '0.0.0.0'
server_port = 31337

server = (server_address, server_port)
sock.bind(server)
print("Listening on " + server_address + ":" + str(server_port))

while True:
    payload, client_address = sock.recvfrom(1024)
    sent = sock.sendto(payload, client_address)
