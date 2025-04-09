from enum import Enum
from re import match

import constants


class ProtocolMessages(Enum):
    MSG_START_GAME = 1
    MSG_HIT = 2
    MSG_MISS = 3
    MSG_JOINED_MATCHMAKING = 4
    MSG_END_GAME = 5
    MSG_BAD_REQUEST = 6
    MSG_YOUR_TURN = 7


class Protocol:
    @classmethod
    def build_shoot_msg(cls, client):
        print(f"Es el turno del jugador: {client.game.current_player}")
        if client.player == client.game.current_player:
            shot_protocol_msg = Protocol.shoot_msg()
            if shot_protocol_msg:
                client.sockfd.send(shot_protocol_msg.encode("ascii"))
            else:
                client.sockfd.send("Input error".encode("ascii"))

    @classmethod
    def shoot_msg(cls):
        coordinate = input("\n").upper().strip()
        expected_input = constants.EXPECTED_INPUT

        # Matches the input with the expected input which should take the form: {A-J}{1-10}
        matched = match(expected_input, coordinate)
        if not matched:
            print("Invalid input, type a coordinate from A-J and a number from 1-10")
        return f"SHOT {coordinate[0]}{coordinate[1:]}"
