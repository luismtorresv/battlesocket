import socket
import sys

import constants
from game import Game
from protocol import ProtocolMessages, Send


class Client:
    def __init__(self):
        self.game = Game()
        self.username = None
        self.email = None
        self.sockfd = None

    def init_socket(self):
        hostname = socket.gethostname()
        server_ip = socket.gethostbyname(hostname)
        addr = (server_ip, constants.PORT)

        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        except OSError:
            print("error: failed to initialise socket.")
            sys.exit(1)

        try:
            sock.connect(addr)
        except OSError:
            print("error: failed to connect to server.")
            sys.exit(1)

        return sock

    def run(self):
        self.sockfd = self.init_socket()
        Send.send_join_msg(self)
        queue = []  #  FIFO queue.

        try:
            while True:
                # Receives messages in the form of a stream of data.
                message = self.sockfd.recv(1024).decode("ascii")
                if not message:
                    print("error: failed to receive data.")
                    return

                queue.append(message.split(constants.TERMINATOR))
                current_message = queue.pop()[0]
                self.read_message(current_message)
                if self.game and self.game.has_ended:
                    break
            self.cleanup()  # Closes the socket
        except (KeyboardInterrupt, EOFError):
            Send.send_surrender_msg(self)

    def cleanup(self):
        # We use `close` instead of `shutdown` because:
        # 1. `close` destroys the socket,
        # 2. and `shutdown` prevents creating new sockets.
        self.sockfd.close()

    def read_message(self, message):
        prot_message = message.split(" ")[0]
        prot_message = ProtocolMessages[f"MSG_{prot_message}"]

        if prot_message not in ProtocolMessages:
            print(f"error: {prot_message} not a valid protocol message.")
            return

        match prot_message:
            case ProtocolMessages.MSG_START_GAME:
                self.game.start_game(message)
                print(f"You are player {self.game.player_letter}.")
                self.game.print_boards()
            case ProtocolMessages.MSG_HIT | ProtocolMessages.MSG_MISS:
                self.game.was_hit(message, self.game.player_letter)
                self.game.print_boards()
            case ProtocolMessages.MSG_JOINED_MATCHMAKING:
                self.init_matchmaking()
            case ProtocolMessages.MSG_END_GAME:
                self.game.end_game(message)
                play_again = input("Do you want to play again? (y/n) ").lower()
                if play_again != "y":
                    return
                self.find_new_game()
            case ProtocolMessages.MSG_TURN:
                self.game.set_current_turn(message)
                self.game.fire_shot(self)

    def find_new_game(self):
        self.cleanup()
        self.game = Game()
        self.run()

    def init_matchmaking(self):
        print("Awaiting players...")
