import socket
import game as g
import protocol as p

PORT = 8080
cola = []


def init_socket():
    #Inicializar el socket
    SERVER = socket.gethostbyname(socket.gethostname())
    ADDR = (SERVER, PORT)
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(ADDR)

    while True:
        #Recibe mensajes del servidor
        mensaje = client.recv(1024).decode("ascii")
        if not mensaje:
            print("Error")
            return
        else:
            cola.append(mensaje.split("\n"))
            
        current_message = cola.pop()[0] #La cola contiene el mensaje junto con la cadena vacia. Solo seleccionamos el mensaje.
        response = p.read_message(current_message)
        



if __name__ == "__main__":
    g.start_client()
    init_socket()

    