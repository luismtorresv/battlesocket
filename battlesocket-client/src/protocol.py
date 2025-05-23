import logging
import sys
from enum import Enum


class ProtocolMessages(Enum):
    MSG_START_GAME = 1
    MSG_HIT = 2
    MSG_MISS = 3
    MSG_JOINED_MATCHMAKING = 4
    MSG_END_GAME = 5
    MSG_TURN = 6
    MSG_BAD_REQUEST = 7


class Protocol:
    MESSAGE_TERMINATOR = "\n"

    @classmethod
    def get_messages(cls, buffer):
        return buffer.split(Protocol.MESSAGE_TERMINATOR)

    @classmethod
    def build_join_msg(cls):
        return f"JOIN{Protocol.MESSAGE_TERMINATOR}"

    @classmethod
    def build_shoot_msg(cls, coordinate):
        return f"SHOT {coordinate[0]}{coordinate[1:]}{Protocol.MESSAGE_TERMINATOR}"

    @classmethod
    def build_input_err_msg(cls):
        return f"BAD_REQUEST INPUT_ERROR{Protocol.MESSAGE_TERMINATOR}"

    @classmethod
    def build_surrender_msg(cls):
        return f"SURRENDER{Protocol.MESSAGE_TERMINATOR}"


class Send:
    @classmethod
    def __send(cls, client, message):
        encoding = "ascii"
        try:
            client.sockfd.send(message.encode(encoding))
        except ConnectionError:
            logging.error("Failed to send.")
            sys.exit(1)

    @classmethod
    def send_shoot_msg(cls, client, message):
        Send.__send(client, Protocol.build_shoot_msg(message))

    @classmethod
    def send_input_err_msg(cls, client):
        Send.__send(client, Protocol.build_input_err_msg())

    @classmethod
    def send_surrender_msg(cls, client):
        Send.__send(client, Protocol.build_surrender_msg())

    @classmethod
    def send_join_msg(cls, client):
        Send.__send(client, Protocol.build_join_msg())
