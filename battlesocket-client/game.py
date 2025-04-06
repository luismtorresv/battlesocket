import time
import os

class Board:
    #el codigo de init y str fueron sacadas de la siguiente implementación: https://stackoverflow.com/questions/77575338/battleship-project-with-python

    def __init__(self, map_size):
        self.board = [["_"] * map_size for _ in range(map_size)]
    
    def __str__(self):
        return "\n".join(" ".join(row) for row in self.board)
    
    def place_coor(self,set_of_coordinates, ship_type):
        for coor in set_of_coordinates:
            self.board[coor[0]][coor[1]] = ship_type

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
    vocab = {'A':1,'B':2,'C':3,'D':4,'E':5,   #TODO: Create dedicated file for constants such as this
            'F':6,'G':7,'H':8,'I':9,'J':10}
        
    new_set = []
    for each in set_of_coordinates.split(" "):    #No me gusta la complejidad pero en este momento no encuentro solución.
        letter,number = each[0],each[1:]  
        letter = vocab[letter]
        new_set.append((letter-1,int(number)-1)) #Se le resta 1 para que quede bien en la board
    return new_set

#Esta funcion espera el mensaje que contiene unicamente el board. 
def place_ships(board):
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

def start_game(msg):
    pivot = msg.find("board:")
    print(pivot)
    headers,board = msg[0:pivot-1], msg[pivot+6:-1]  #Separar los 'headers' del mensaje de el board usando el ultimo header como pivote.
    name, start_time, initial_player = headers.split(" ")
    player_board = place_ships(board)
    return (start_time,initial_player,player_board)
