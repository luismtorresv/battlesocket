import socket
import game as g
import protocol as p
import constants as c

class Status:
    def __init__(self):
        self.game = None
        self.player = None

    def set_player(self, player):
        self.player = player
    
    def set_game(self,game):
        self.game = game

def init_socket():
    SERVER_IP = socket.gethostbyname(socket.gethostname())
    ADDR = (SERVER_IP, c.PORT)
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(ADDR)
    return client

def read_message(msg,status):
    prot_message = msg.split(" ")[0] 
    prot_message = p.Protocol[f'MSG_{prot_message}']
    print(msg)
    match prot_message:
        case p.Protocol.MSG_START_GAME:
            status.set_game(g.start_game(msg))
            status.game.print_boards()
        case p.Protocol.MSG_HIT:
            #recibio un hit
            status.game.place_hit_or_miss(msg,status.player)
            status.game.print_boards()
        case p.Protocol.MSG_MISS:
            #Recibio un miss
            status.game.place_hit_or_miss(msg,status.player)
            status.game.print_boards()
        case p.Protocol.MSG_JOINED_MATCHMAKING:
            #Recibio un inicio de conexion
            player = p.init_matchmaking(msg)
            status.set_player(player)
        case p.Protocol.MSG_END_GAME:
            #Recibio un final de juego
            status.game.status = 'INACTIVE'
        case p.Protocol.MSG_BAD_REQUEST:
            #Recibio un error
            print("Hubo un error del lado del cliente.")

def init_game():
    socket = init_socket()  #Sets up the socket.
    cola = []
    client_status = Status() #Sets up the status of the game

    while True:
        #Recibe mensajes del servidor
        mensaje = socket.recv(1024).decode("ascii")
        if not mensaje:
            print("Error")
            return
        else:
            cola.append(mensaje.split("\n"))
            
        current_message = cola.pop()[0] #La cola contiene el mensaje junto con la cadena vacia. Solo seleccionamos el mensaje.
        read_message(current_message,client_status)
        if client_status.game != None:
            client_status.game.action(socket,client_status)
            if client_status.game.status == 'INACTIVE':
                break
        else: 
            continue
    return socket

def cleanup_sockt(socket):
    socket.shutdown()
    socket.close()

def init_client():
    g.start_client()
    socket = init_game()
    cleanup_sockt(socket)
    #TODO: Close the socket.
