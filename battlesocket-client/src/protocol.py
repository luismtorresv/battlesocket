from enum import Enum


class Protocol(Enum):
    MSG_START_GAME = 1
    MSG_HIT = 2
    MSG_MISS = 3
    MSG_JOINED_MATCHMAKING = 4
    MSG_END_GAME = 5
    MSG_BAD_REQUEST = 6
    # Los mensajes del protocolo no se deberian de poder acceder tan facilmente.


def init_matchmaking(msg):
    player = msg.split(" ")[1]  # TODO: CAMBIAR A UN ESPACIO CUANDO PAREMOS DE USAR |
    print(f"You are player {player}! Awaiting players...")
    return player
