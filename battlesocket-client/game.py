import time
import os
import constants as c

class Board:
    #el codigo de init y str fueron sacadas de la siguiente implementación: https://stackoverflow.com/questions/77575338/battleship-project-with-python

    def __init__(self, map_size):
        self.board = [["_"] * map_size for _ in range(map_size)]
    
    def __str__(self):
        return "\n".join(" ".join(row) for row in self.board)
    
    def place_coor(self,set_of_coordinates, ship_type):
        for coor in set_of_coordinates:
            self.board[coor[0]][coor[1]] = ship_type
    
    def update_board(self,msg_type,coor):
        if msg_type == 'HIT':
            self.board[coor[0]][coor[1]] = 'X'
        elif msg_type == 'MISS':
            self.board[coor[0]][coor[1]] = 'O'


class Game:
    def __init__(self,player_board,start_time,current_player):
        self.player_board = self.place_ships(player_board)
        self.opposite_player_board = Board(10)
        self.has_started = True
        self.start_time = start_time
        self.current_player = current_player
    
    #Esta funcion espera el mensaje que contiene unicamente el board. 
    def place_ships(self, board):
        battleships = board.split(";")
        for i in range(len(battleships)):
            name, coor = battleships[i].split(":")
            coor = translate(coor)

            battleships[i] = (name,coor)
        
        player_board = Board(10)
        for ship in battleships:
            ship[0].strip()
            if ship[0].__contains__('cruiser'):
                player_board.place_coor(ship[1],ship[0][1])  #En el caso del cruiser, se pone minuscula
            else:
                player_board.place_coor(ship[1],ship[0][1].upper()) #La primera letra de cada barco es puesta en el board. TODO: Manejar el whitespace en el mensaje.
        return player_board
    
    def print_boards(self):
        print(f'Opposing Player:\n {self.opposite_player_board} \nYour board: \n{self.player_board}')
    
    def place_hit(msg):
        raise NotImplementedError

    def shoot(self,player):
        print(f'Es el turno del jugador: {self.current_player}')
        coordinate = input("\n")
        coordinate = coordinate.upper()
        if player == self.current_player:
            coordinate.strip()
            return f'SHOT {coordinate[0]}-{coordinate[1]}'
        else:
            print("Porfavor esperar a que sea tu turno")
            return 0


def start_client():
    print("Welcome to Battleship™!")
    while True:
        ans = input("Do you wish to play? (Y/N)")
        if ans.lower() == "y":
            return
        elif ans.lower() == "n":
            print("Goodbye!")
            os.exit(0)
        else:
            print("Please try again!")
    




def translate(set_of_coordinates):
    new_set = []
    for each in set_of_coordinates.split(" "):    #No me gusta la complejidad pero en este momento no encuentro solución.
        letter,number = each[0],each[1:]  
        letter = c.vocab[letter]
        new_set.append((letter-1,int(number)-1)) #Se le resta 1 para que quede bien en la board
    return new_set

def start_game(msg):
    pivot = msg.find("board:")
    headers,board = msg[0:pivot-1], msg[pivot+6:-1]  #Separar los 'headers' del mensaje de el board usando el ultimo header como pivote.
    _, start_time, initial_player = headers.split(" ") #TODO: No se que hacer con el initial_player
    game = Game(board,start_time,initial_player[-1])
    print(f'\nThe first turn goes to: player {initial_player[-1]}')
    return game
