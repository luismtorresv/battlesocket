from enum import Enum

class Protocol(Enum):
    MSG_START_GAME = 1
    MSG_HIT = 2
    MSG_MISS = 3
    MSG_JOINED_MATCHMAKING = 4
    MSG_END_GAME = 5
    MSG_BAD_REQUEST = 6
    #Los mensajes del protocolo no se deberian de poder acceder tan facilmente.


def init_matchmaking(msg):
    print(msg)
    turn = msg.split(" ")[1] #TODO: CAMBIAR A UN ESPACIO CUANDO PAREMOS DE USAR |
    print(f'You are player {turn}! Awaiting players...')

def read_message(msg):
    prot_message = msg.split(" ")[0] #TODO: CAMBIAR A UN ESPACIO CUANDO PAREMOS DE USAR |
    prot_message = Protocol[f'MSG_{prot_message}']
    match prot_message:
        case Protocol.MSG_START_GAME:
            #inicio el juego
            pass
        case Protocol.MSG_HIT:
            #recibio un hit
            pass
        case Protocol.MSG_MISS:
            #Recibio un miss
            pass
        case Protocol.MSG_JOINED_MATCHMAKING:
            #Recibio un inicio de conexion
            init_matchmaking(msg)
            pass
        case Protocol.MSG_END_GAME:
            #Recibio un final de juego
            pass
        case Protocol.MSG_BAD_REQUEST:
            #Recibio un error
            pass
