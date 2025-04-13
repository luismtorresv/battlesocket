import constants
from protocol import Send
from re import match


class Board:
    # Inspired by:  https://stackoverflow.com/questions/77575338/battleship-project-with-python
    def __init__(self, map_size):
        self.board = [["_"] * map_size for _ in range(map_size)]

    def __str__(self):
        return "\n".join(" ".join(row) for row in self.board)

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
        self.start_time = None
        self.current_player = None
        self.player_letter = None
        self.turn_time = None
        self.new_game = False

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
            f"Opposing Board:\n{self.opposite_player_board}\n"
            f"Your Board: \n{self.player_board}"
        )

    def was_hit(self, message, player):
        if "SUNK" in message:
            (
                msg_type,
                coordinate,
                _,
            ) = message.split(" ")
            self.remaining_ships -= 1

            print(f"A ship sunk! There are {self.remaining_ships} ships remainings.")
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
        _, player_letter, time = message.split(
            " "
        )  # TODO: timeout. The turn sends the time.
        self.current_player = player_letter
        self.turn_time = time

    def fire_shot(self, client):
        if client.game.player_letter != client.game.current_player:
            print("Its the other player's turn.")
            return

        print("Its your turn!")    
        matched = False
        while not matched:
            try:
                coordinate = input("\n").upper().strip()
            except KeyboardInterrupt:
                Send.send_surrender_msg(client)
                return
            
            if "NEW GAME" in coordinate.upper().strip(): 
                Send.send_surrender_msg(client)
                client.find_new_game()
                self.new_game = True
                return
                

            expected_input = constants.EXPECTED_INPUT

            # Matches the input with the expected input which should take the form: {A-J}{1-10}
            matched = match(expected_input, coordinate)
            if not matched:

                print(
                    "Invalid input, type a coordinate from A-J and a number from 1-10"
                )


        Send.send_shoot_msg(client, coordinate)

    def surrender(self):
        self.has_ended = True
        if self.current_player != self.player_letter:
            print("The opponent has forfit the match. You Won!")
        else:
            print("You gave up...")

    def start_game(self, message):
        # Uses the word 'board' to split the control information into 2 sides.
        message = message.split(" ", maxsplit=2)
        _, player_letter, board = message
        board = board.strip("{}")
        self.place_ships(board)
        self.player_letter = player_letter


    def end_game(self, message):
        if " " not in message:
            print("The game ended.")
            return

        elif "SURRENDER" in message:
            self.surrender()
            return
        
        _, player_letter = message.split(" ")
        print(f"Game Over. The winner is: Player {player_letter}")
