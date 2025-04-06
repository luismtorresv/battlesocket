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
        response = read_message(current_message)
        

def read_message(msg):
    msg = msg.replace('|',' ') #TODO: CAMBIAR A UN ESPACIO CUANDO PAREMOS DE USAR |. Esta linea es provisional.
    prot_message = msg.split(" ")[0] 
    prot_message = p.Protocol[f'MSG_{prot_message}']
    match prot_message:
        case p.Protocol.MSG_START_GAME:
            data = g.start_game(msg)
            print(data[2])
        case p.Protocol.MSG_HIT:
            #recibio un hit
            pass
        case p.Protocol.MSG_MISS:
            #Recibio un miss
            pass
        case p.Protocol.MSG_JOINED_MATCHMAKING:
            #Recibio un inicio de conexion
            p.init_matchmaking(msg)
            pass
        case p.Protocol.MSG_END_GAME:
            #Recibio un final de juego
            pass
        case p.Protocol.MSG_BAD_REQUEST:
            #Recibio un error
            pass

def init_client():
    g.start_client()
    init_socket()
    #TODO: Close the socket.
