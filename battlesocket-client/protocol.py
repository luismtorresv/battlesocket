#Los mensajes del protocolo no se deberian de poder acceder tan facilmente. Definitivamente cambiar.
protocol_messages = {"MSG_START_GAME":1,"MSG_HIT":2, "MSG_MISS":3,   
                     "MSG_JOINED_MATCHMAKING":4, "MSG_END_GAME":5,"MSG_BAD_REQUEST":6}

def init_matchmaking(msg):
    print(msg)
    turn = msg.split(" ")[1]
    print(f'You are player {turn}! Awaiting players...')

def read_message(msg):
    #Decidir si el mensaje va a ser separado por un '|' o un espacio

    prot_message = msg.split(" ")[0] #PORFAVOR CAMBIAR A UN ESPACIO CUANDO PAREMOS DE USAR |

    prot_message = protocol_messages[f'MSG_{prot_message}'] #Se asocia con el vocabulario
    match prot_message:
        case 1:
            #inicio el juego
            pass
        case 2: 
            #recibio un hit
            pass
        case 3:
            #Recibio un miss
            pass
        case 4:
            #Recibio un inicio de conexion
            init_matchmaking(msg)
            pass
        case 5:
            #Recibio un final de juego
            pass
        case 6: 
            #Recibio un error
            pass
