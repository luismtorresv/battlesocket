from constants import Constants


class Board:
    # Inspired by:  https://stackoverflow.com/questions/77575338/battleship-project-with-python
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

    def place_ships(self, board):
        battleships = board.split(";")  # Stores all the battleships

        for i in range(len(battleships)):
            name, coor = battleships[i].split(":")
            coor = translate(coor)

            battleships[i] = (name, coor)

        player_board = Board(10)
        for ship in battleships:
            ship_name = ship[0].strip()
            ship_letter = ship_name[0]
            ship_coordinates = ship[1]

            if ship_name.__contains__("cruiser"):
                player_board.place_coor(
                    ship_coordinates, ship_letter
                )  # for cruisers, a lowercase c is placed.
            else:
                player_board.place_coor(ship_coordinates, ship_letter.upper())

        return player_board, len(battleships)

    def print_boards(self):
        print(
            f"Opposing Board:\n{self.opposite_player_board}\n"
            f"Your Board: \n{self.player_board}"
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

            print(msg_type)  # Prints HIT or MISS
            self.place_hit_or_miss(coordinate, letter, player, msg_type)

        self.change_current_player()

    def place_hit_or_miss(self, coordinate, letter, player, msg_type):
        # translate returns a list of size 1.
        translated_coor = translate(coordinate)[0]
        # If the messages letter is different from the current players
        if letter != player:
            self.opposite_player_board.update_board(msg_type, translated_coor)
        else:
            if msg_type != "MISS":
                self.player_board.update_board(msg_type, translated_coor)

    def change_current_player(self):
        self.current_player = "B" if self.current_player == "A" else "A"


def translate(set_of_coordinates):
    new_set = []
    for each in set_of_coordinates.split(" "):
        letter, number = each[0], each[1:]
        letter = Constants.VOCAB[letter]

        # Substract 1 from each coordinate so it complies with python logic.
        new_set.append((letter - 1, int(number) - 1))
    return new_set


def start_game(msg):
    pivot = msg.find("board:")
    # Uses the word 'board' to split the control information into 2 sides.
    control_info, board = (
        msg[0 : pivot - 1],
        msg[pivot + 7 : -1],
    )
    _, start_time, initial_player = control_info.split(" ")
    game = Game(board, start_time, initial_player[-1])
    print(f"\nPlayer {initial_player[-1]} goes first.")
    return game


def end_game(msg):
    if msg.__contains__(" "):
        _, winner = msg.split(" ")
        print(f"The winner is: Player: {winner}")
    else:
        print("The game ended.")
