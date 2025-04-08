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
    #Inicializar el socket
    cola = []
    client_status = Status() #TODO: Change this.

    SERVER = socket.gethostbyname(socket.gethostname())
    ADDR = (SERVER, c.PORT)
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
        read_message(current_message,client_status)
        if client_status.game != None:
            client_status.game.action(client,client_status)
        else: 
            continue

def read_message(msg,status):
    msg = msg.replace('|',' ') #TODO: CAMBIAR A UN ESPACIO CUANDO PAREMOS DE USAR |. Esta linea es provisional.
    prot_message = msg.split(" ")[0] 
    prot_message = p.Protocol[f'MSG_{prot_message}']
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
            pass
        case p.Protocol.MSG_BAD_REQUEST:
            #Recibio un error
            pass

def init_client():
    g.start_client()
    init_socket()
    #TODO: Close the socket.
