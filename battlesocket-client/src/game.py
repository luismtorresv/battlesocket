import constants as c
import re as r


class Board:
    # el codigo de init y str fueron sacadas de la siguiente implementación: https://stackoverflow.com/questions/77575338/battleship-project-with-python

    def __init__(self, map_size):
        self.board = [["_"] * map_size for _ in range(map_size)]

    def __str__(self):
        return "\n".join(" ".join(row) for row in self.board)

    def place_coor(self, set_of_coordinates, ship_type):
        for coor in set_of_coordinates:
            self.board[coor[0]][coor[1]] = ship_type

    def update_board(self, msg_type, coor):
        if msg_type == "HIT":
            self.board[coor[0]][coor[1]] = "X"
        elif msg_type == "MISS":
            self.board[coor[0]][coor[1]] = "O"


class Game:
    def __init__(self, player_board, start_time, current_player):
        self.player_board, self.remaining_ships = self.place_ships(player_board)
        self.opposite_player_board = Board(10)
        self.status = "ACTIVE"
        self.start_time = start_time
        self.current_player = current_player

    # Esta funcion espera el mensaje que contiene unicamente el board.
    def place_ships(self, board):
        battleships = board.split(";")  # Stores all the battleships
        for i in range(len(battleships)):
            name, coor = battleships[i].split(":")
            coor = translate(coor)

            battleships[i] = (name, coor)

        player_board = Board(10)
        for ship in battleships:
            ship[0].strip()
            if ship[0].__contains__("cruiser"):
                player_board.place_coor(
                    ship[1], ship[0][1]
                )  # En el caso del cruiser, se pone minuscula
            else:
                player_board.place_coor(ship[1], ship[0][1].upper())
        return player_board, len(battleships)

    def print_boards(self):
        print(
            f"Opposing Player:\n{self.opposite_player_board} \nYour board: \n{self.player_board}"
        )

    def was_hit(self, msg, player):
        if msg.__contains__("SUNK"):
            msg_type, coordinate, _, turn = msg.split(" ")
            letter = turn[-1]  # Ignores 'Next:', only grabs the player
            self.remaining_ships -= 1

            print(f"¡Se ha hundido un barco! Quedan {self.remaining_ships} barcos.")
            self.place_hit_or_miss(coordinate, letter, player, msg_type)

        else:
            msg_type, coordinate, turn = msg.split(" ")
            letter = turn[-1]

            print(f"{msg_type} en la coordenada: {coordinate}")
            self.place_hit_or_miss(coordinate, letter, player, msg_type)

        self.change_current_player()

    def place_hit_or_miss(self, coordinate, letter, player, msg_type):
        translated_coor = translate(coordinate)[
            0
        ]  # translate returns a list of size 1.
        if (
            letter != player
        ):  # If the messages letter is different from the current players
            self.opposite_player_board.update_board(msg_type, translated_coor)
        else:
            if msg_type != "MISS":
                self.player_board.update_board(msg_type, translated_coor)

    def shoot(self):
        coordinate = input("\n")
        coordinate = coordinate.upper()
        coordinate.strip()

        expected_input = c.expected_input
        match = r.match(expected_input, coordinate)
        if match:
            return f"SHOT {coordinate[0]}{coordinate[1]}"
        else:
            print("Invalid input, type a coordinate from A-J and a number from 1-10")

    def change_current_player(self):
        self.current_player = "B" if self.current_player == "A" else "A"

    def action(self, client, client_status):
        print(f"Es el turno del jugador: {self.current_player}")
        if client_status.player == self.current_player:
            SHOT_protocol_msg = self.shoot()
            if SHOT_protocol_msg:
                client.send(SHOT_protocol_msg.encode("ascii"))
            else:
                client.send("Input error".encode("ascii"))
        else:
            return


    def handle_bad_request(self,msg):
        print("There was an error on the client side.")
        _,player = msg.split(" ")
        player = player[-1]  #The letter is found at the end of the string
        self.change_current_player()




def start_client():
    print("Welcome to Battleship™!")
    while True:
        ans = input("Do you wish to play? (Y/N)")
        if ans.lower() == "y":
            return
        else:
            print("Goodbye!")
            from sys import exit

            exit(1)


def translate(set_of_coordinates):
    new_set = []
    for each in set_of_coordinates.split(
        " "
    ):  # No me gusta la complejidad pero en este momento no encuentro solución.
        letter, number = each[0], each[1:]
        letter = c.vocab[letter]
        new_set.append(
            (letter - 1, int(number) - 1)
        )  # Se le resta 1 para que quede bien en la board
    return new_set


def start_game(msg):
    pivot = msg.find("board:")
    headers, board = (
        msg[0 : pivot - 1],
        msg[pivot + 6 : -1],
    )  # Separar los 'headers' del mensaje de el board usando el ultimo header como pivote.
    _, start_time, initial_player = headers.split(" ")
    game = Game(board, start_time, initial_player[-1])
    print(f"\nPlayer {initial_player[-1]} goes first.")
    return game
