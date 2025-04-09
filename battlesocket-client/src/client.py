import socket
import game as g
from protocol import Protocol
from constants import Constants
from sys import exit


class Client:
    def __init__(self):
        self.game = None
        self.player = None
        self.username, self.email = self.start_client()

    def set_player(self, player):
        self.player = player

    def set_game(self, game):
        self.game = game

    def start_client(self):
        print("Welcome to Battleship™!")
        while True:
            ans = input("Do you wish to play? (Y/N)\n")
            if ans.lower() == "y":
                username, email = Client.inp_user_data()
                return username, email
            else:
                print("Goodbye!")
                exit(1)

    def inp_user_data():  # Inputs for the user data
        username = input("Please input a username: \n")
        email = input("Please input your email: \n")
        return username, email


def init_socket():
    SERVER_IP = socket.gethostbyname(socket.gethostname())
    ADDR = (SERVER_IP, Constants.PORT)
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(ADDR)
    return client


def read_message(msg, client, socket):
    prot_message = msg.split(" ")[0]
    prot_message = Protocol[f"MSG_{prot_message}"]

    match prot_message:
        case Protocol.MSG_START_GAME:
            client.set_game(g.start_game(msg))
            client.game.print_boards()
        case Protocol.MSG_HIT:
            client.game.was_hit(msg, client.player)
            client.game.print_boards()

        case Protocol.MSG_MISS:
            client.game.was_hit(msg, client.player)
            client.game.print_boards()

        case Protocol.MSG_JOINED_MATCHMAKING:
            player = init_matchmaking(msg)
            client.set_player(player)

        case Protocol.MSG_END_GAME:
            client.game.status = "INACTIVE"
            g.end_game(msg)

        case Protocol.MSG_BAD_REQUEST:
            # TODO: This response
            print("Se mando un bad request")

        case Protocol.MSG_YOUR_TURN:
            client.game.current_player = client.player
            Protocol.build_shoot_msg(socket, client)


def run_client(client):
    socket = init_socket()  # Sets up the socket.
    cola = []

    while True:
        mensaje = socket.recv(1024).decode(
            "ascii"
        )  # Recieves messages in the form of a stream of data.
        if not mensaje:
            print("Error")
            return
        else:
            cola.append(mensaje.split("\n"))
        # The first message in the queue is the first one to be answered.
        current_message = cola.pop()[0]
        read_message(current_message, client, socket)
        if client.game:
            if client.game.status == "INACTIVE":
                break
        else:
            continue
    return socket


def cleanup_sockt(socket):
    socket.shutdown(1)
    socket.close()


def init_client():
    client = Client()  # Sets up the status of the game.
    socket = run_client(client)
    cleanup_sockt(socket)  # Closes the socket


def handle_bad_request(self, msg):
    # TODO: Add this to the log.
    _, player = msg.split(" ")
    player = player[-1]  # The letter is found at the end of the string
    self.change_current_player()


def init_matchmaking(msg):
    player = msg.split(" ")[1]
    print(f"You are player {player}! Awaiting players...")
    return player
