import logging
import re
import textwrap

import constants
from inputimeout import SocketReadAvailable, TimeoutOcurred, inputimeout
from protocol import Send


class Board:
    # Inspired by:  https://stackoverflow.com/questions/77575338/battleship-project-with-python
    def __init__(self, map_size):
        self.map_size = map_size
        self.board = [["_"] * map_size for _ in range(map_size)]

    def __str__(self):
        # Asked chat gpt for help labeling the board as such:

        # Create the header with column numbers 1–10
        header = "  " + " ".join(f"{i+1:2}" for i in range(self.map_size))

        # Create the board rows with A–J labels
        rows = []
        for i, row in enumerate(self.board):
            row_label = chr(ord("A") + i)
            row_str = f"{row_label}  " + "  ".join(row)
            rows.append(row_str)

        return header + "\n" + "\n".join(rows)

    def place_ship(self, list_coordinates, ship_type):
        for coordinate in list_coordinates:
            letter, number = coordinate
            self.board[letter][number] = ship_type

    def update_board(self, msg_type, coordinates):
        marker = ""
        match msg_type:
            case "HIT":
                marker = "X"
            case "MISS":
                marker = "O"
        letter, number = coordinates
        self.board[letter][number] = marker


class Game:
    def __init__(self):
        self.player_board = Board(constants.BOARD_SIZE)
        self.remaining_ships = None
        self.opposite_player_board = Board(constants.BOARD_SIZE)
        self.has_ended = False
        self.current_player = None
        self.player_letter = None
        self.turn_time = None

    def place_ships(self, board):
        battleships_str = board.split(";")  # Stores all the battleships
        battleships_str = battleships_str[:-1]  # Last one is empty space
        battleships = []

        for ship in battleships_str:
            name, coordinates = ship.split(":")
            coordinates = self.translate(coordinates)
            battleships.append((name, coordinates))

        self.remaining_ships = len(battleships)
        for ship in battleships:
            ship_name = ship[0].strip()
            ship_letter = ship_name[0]
            ship_coordinates = ship[1]

            # For cruiser ships, a lowercase 'c' is placed.
            if "cruiser" not in ship_name:
                ship_letter = ship_letter.upper()

            self.player_board.place_ship(ship_coordinates, ship_letter)

    def print_boards(self):
        print(
            "{:^30}".format("Opposing board"),
            f"\n{self.opposite_player_board}",
            "\n",
            "{:^30}".format("Your board"),
            f"\n{self.player_board}",
            sep="",
        )

    def was_hit(self, message, player):
        if "SUNK" in message:
            (
                msg_type,
                coordinate,
                _,
            ) = message.split(" ")
            self.remaining_ships -= 1

            print(f"A ship sunk! There are {self.remaining_ships} ships remaining.")
            self.place_hit_or_miss(coordinate, player, msg_type)
        else:
            msg_type, coordinate = message.split(" ")

            print(msg_type)  # Prints HIT or MISS
            self.place_hit_or_miss(coordinate, player, msg_type)

        self.change_current_player()

    def place_hit_or_miss(self, coordinate, player, msg_type):
        # translate returns a list of size 1.
        translated_coor = self.translate(coordinate)[0]
        # If the messages letter is different from the current players
        if self.current_player == player:
            self.opposite_player_board.update_board(msg_type, translated_coor)
        elif msg_type != "MISS":
            self.player_board.update_board(msg_type, translated_coor)

    def change_current_player(self):
        self.current_player = "B" if self.current_player == "A" else "A"

    def translate(self, set_of_coordinates):
        new_set = []
        for coord in set_of_coordinates.split(" "):
            letter, number = coord[0], coord[1:]
            letter = constants.VOCAB[letter]

            # Substract 1 from each coordinate so it complies with python logic.
            new_set.append((letter - 1, int(number) - 1))
        return new_set

    def set_current_turn(self, message):
        # TODO: timeout. The turn sends the time.
        _, player_letter, time = message.split(" ")
        self.current_player = player_letter
        self.turn_time = time

    def fire_shot(self, client):
        # The expected input is a letter from A-J concatenated with a number from 1-10.
        expected_input = re.compile(r"^[A-J]([1-9]|10)$", flags=re.ASCII)
        matched = False
        while not matched:
            try:
                coordinate = inputimeout(client.sockfd, "Coordinate: ")
                coordinate = coordinate.upper().strip()

                matched = re.match(expected_input, coordinate)
                if not matched:
                    print(
                        "Invalid input.\n"
                        "Type a coordinate from A-J and a number from 1-10"
                        " (e.g. D9 or A10)"
                    )
                else:
                    logging.info("Fired a shot at %s.", coordinate)
                    Send.send_shoot_msg(client, coordinate)
            except SocketReadAvailable:
                # HACK: We're not actually checking whether the server sent an
                # `END_GAME` message due to a timeout.
                print("You took too long to play. You lost.")
                client.game.has_ended = True
                return
            except TimeoutOcurred:
                logging.error(
                    "User took too long to play, but server didn't send anything"
                )
                client.cleanup()
                sys.exit(1)

    def start_game(self, message):
        # Uses the word 'board' to split the control information into 2 sides.
        message = message.split(" ", maxsplit=2)
        _, player_letter, board = message
        board = board.strip("{}")
        self.place_ships(board)
        self.player_letter = player_letter

    def end_game(self, message):
        self.has_ended = True

        pattern = re.compile(
            textwrap.dedent(
                r"""
                ^  # Start of line
                END_GAME
                \s
                (?P<reason>WINNER|SURRENDER|DISCONNECTION|TIMEOUT)  # Why did the game end?
                \s
                (?P<player>A|B) # Who won? (Or surrendered?)
                $ # End of line
                """
            ),
            flags=re.VERBOSE,
        )
        match = re.match(pattern, message)
        if not match:
            logging.error('Couldn\'t parse END_GAME message "%s"', message)
            return

        parsed_message = match.groupdict()
        match parsed_message["reason"]:
            case "SURRENDER":
                if self.player_letter == parsed_message["player"]:
                    logging.debug("The client surrendered.")
                    print("You gave up... Your opponent wins...")
                else:
                    print("The opponent has surrendered the match. You won!")
            case "WINNER":
                print(f"Game over! Player {parsed_message['player']} won.")
            case "DISCONNECTION":
                print("Your opponent has disconnected. So... you win?")
            case "TIMEOUT":
                if self.player_letter == parsed_message["player"]:
                    print("You didn't send your shot on time. You lost.")
                else:
                    print("Your opponent didn't send their turn in time. You win!")
