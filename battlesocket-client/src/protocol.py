from enum import Enum
from constants import Constants
from re import match


class Protocol(Enum):
    MSG_START_GAME = 1
    MSG_HIT = 2
    MSG_MISS = 3
    MSG_JOINED_MATCHMAKING = 4
    MSG_END_GAME = 5
    MSG_BAD_REQUEST = 6
    MSG_YOUR_TURN = 7

    def build_shoot_msg(socket, client):
        game = client.game
        print(f"Es el turno del jugador: {game.current_player}")
        if client.player == game.current_player:
            SHOT_protocol_msg = Protocol.shoot_msg()
            if SHOT_protocol_msg:
                socket.send(SHOT_protocol_msg.encode("ascii"))
            else:
                socket.send("Input error".encode("ascii"))
        else:
            return

    def shoot_msg():
        coordinate = input("\n").upper().strip()
        expected_input = Constants.EXPECTED_INPUT

        # Matches the input with the expected input which should take the form: {A-J}{1-10}
        matched = match(expected_input, coordinate)
        if matched:
            return f"SHOT {coordinate[0]}{coordinate[1:]}"
        else:
            print("Invalid input, type a coordinate from A-J and a number from 1-10")
