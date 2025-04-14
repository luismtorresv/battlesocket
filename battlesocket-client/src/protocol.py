from enum import Enum
from constants import TERMINATOR

class ProtocolMessages(Enum):
    MSG_START_GAME = 1
    MSG_HIT = 2
    MSG_MISS = 3
    MSG_JOINED_MATCHMAKING = 4
    MSG_END_GAME = 5
    MSG_TURN = 6


class Protocol:
    # Dollar signs are used to indicate the end of the message
    @classmethod
    def build_shoot_msg(cls, coordinate):
        return f"SHOT {coordinate[0]}{coordinate[1:]}{TERMINATOR}"

    @classmethod
    def build_input_err_msg(cls):
        return f"BAD_REQUEST INPUT_ERROR{TERMINATOR}"

    @classmethod
    def build_surrender_msg(cls):
        return f"SURRENDER{TERMINATOR}"


class Send:
    @classmethod
    def send_shoot_msg(cls, client, message):
        protocol_message = Protocol.build_shoot_msg(message)
        client.sockfd.send(protocol_message.encode("ascii"))

    @classmethod
    def send_input_err_msg(cls, client):
        protocol_message = Protocol.build_input_err_msg()
        client.sockfd.send(protocol_message.encode("ascii"))

    @classmethod
    def send_surrender_msg(cls, client):
        protocol_message = Protocol.build_surrender_msg()
        client.sockfd.send(protocol_message.encode("ascii"))
