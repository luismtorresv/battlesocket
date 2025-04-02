import socket

PORT = 8080
SERVER = socket.gethostbyname(socket.gethostname())
ADDR = (SERVER,PORT)
DISCONNECT_MESSAGE = "BAD_REQUEST|\n"

client = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
client.connect(ADDR)


while True:
    mensaje = client.recv(1024)
    if mensaje == DISCONNECT_MESSAGE:
        break
    else:
        print(mensaje)

    