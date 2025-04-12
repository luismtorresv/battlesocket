from enum import Enum


class ProtocolMessages(Enum):
    MSG_START_GAME = 1
    MSG_HIT = 2
    MSG_MISS = 3
    MSG_JOINED_MATCHMAKING = 4
    MSG_END_GAME = 5
    MSG_BAD_REQUEST = 6
    MSG_TURN = 7


class Protocol:
    @classmethod
    def build_shoot_msg(cls, coordinate):
        return f"SHOT {coordinate[0]}{coordinate[1:]}"

    @classmethod
    def build_input_err_msg(cls):
        return "BAD_REQUEST INPUT_ERROR"
    
    """ @classmethod
    def build_surrender_msg(cls):
        return f"END_GAME SURRENDER" """

class Send:
    @classmethod
    def send_shoot_msg(cls,client,message):
        protocol_message = Protocol.build_shoot_msg(message)
        client.sockfd.send(protocol_message.encode("ascii"))

    @classmethod
    def send_input_err_msg(cls,client):
        protocol_message = Protocol.build_input_err_msg()
        client.sockfd.send(protocol_message.encode("ascii"))

    """ def send_surrender_msg(cls,client):
        protocol_message = Protocol.build_surrender_msg()
        client.sockfd.send(protocol_message.encode("ascii")) """
