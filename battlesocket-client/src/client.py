import logging
import socket
import sys

import constants
from game import Game
from protocol import ProtocolMessages, Send


class Client:
    def __init__(self, ip, port):
        self.game = Game()
        self.username = None
        self.email = None
        self.sockfd = None
        self.server_addr = (ip, port)

    def init_socket(self):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        except OSError:
            logging.error("Failed to initialise socket.")
            sys.exit(1)

        try:
            sock.connect(self.server_addr)
        except OSError:
            logging.error("Failed to connect to server at %s.", self.server_addr)
            sys.exit(1)

        logging.info("Client connected to server at %s.", self.server_addr)
        return sock

    def run(self):
        self.sockfd = self.init_socket()
        Send.send_join_msg(self)
        try:
            queue = []  #  FIFO queue.
            while True:
                # Receives messages in the form of a stream of data.
                buffer = self.sockfd.recv(1024).decode("ascii")
                if not buffer:
                    logging.error("error: server has disconnected.")
                    return

                messages = buffer.split(constants.TERMINATOR)
                queue.extend(messages)
                while queue:
                    current_message = queue.pop(0)  # First element. It's FIFO.
                    if current_message != "":
                        self.read_message(current_message)
                        if self.game and self.game.has_ended:
                            return
        except (KeyboardInterrupt, EOFError):
            print("You gave up... Your opponent wins...")
            Send.send_surrender_msg(self)
            return
        except ConnectionError:
            logging.error("Server disconnected.")
            print("It seems that the server has disconnected. Exiting...")
            sys.exit(1)

    def cleanup(self):
        # We use `close` instead of `shutdown` because:
        # 1. `close` destroys the socket,
        # 2. and `shutdown` prevents creating new sockets.
        self.sockfd.close()
        logging.info("Closed connection with %s.", self.server_addr)

    def read_message(self, message):
        prot_message = message.split(" ")[0]
        try:
            prot_message = ProtocolMessages[f"MSG_{prot_message}"]
        except KeyError:
            logging.error('"%s" is not a valid protocol message.', prot_message)
            return

        logging.info('Received message: "%s".', message)
        match prot_message:
            case ProtocolMessages.MSG_START_GAME:
                self.game.start_game(message)
                logging.info("Game has started.")
                print(f"You are player {self.game.player_letter}.")
            case ProtocolMessages.MSG_HIT | ProtocolMessages.MSG_MISS:
                self.game.was_hit(message, self.game.player_letter)
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
                self.game.print_boards()
                if self.game.player_letter != self.game.current_player:
                    print("It's the other player's turn.")
                    return
                else:
                    print("It's your turn! You have 30 seconds to place your shot!")
                self.game.fire_shot(self)
            case ProtocolMessages.MSG_BAD_REQUEST:
                # Ignore bad requests. Just log them.
                logging.error("Client got bad request message.")

    def find_new_game(self):
        logging.info("User chose to find new game.")
        self.cleanup()
        self.game = Game()
        self.run()

    def init_matchmaking(self):
        logging.info("Waiting for other players.")
        print("Awaiting players...")
