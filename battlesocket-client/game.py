import time
import os

#Create y display battlefield fueron sacadas de la siguiente implementación: https://stackoverflow.com/questions/77575338/battleship-project-with-python
def create_battlefield(map_size):
    """
    function to create a map based on size
    """
    return [["_"] * map_size for _ in range(map_size)]

def display_battlefield(board):
    """
    function to display current state of the map.
    """
    for row in board:
        print(" ".join(row))

def start_client():
    print("Welcome to Battleship™!")
    time.sleep(1)
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
    vocab = {'A':1,'B':2,'C':3,'D':4,'E':5,
            'F':6,'G':7,'H':8,'I':9,'J':10}
        
    new_set = []
    for each in set_of_coordinates.split(" "):    #No me gusta la complejidad pero en este momento no encuentro solución.
        letter,number = each[0]-1,int(each[1:])-1  #Se le resta 1 para que quede acorde a las posiciones con las que python funciona. 
        letter = vocab[letter]
        new_set.append((letter,number))
    return new_set
            


#Esta funcion espera el mensaje que contiene unicamente el board. 
def place_ships(board):
    battleships = board.split(";")
    for i in range(len(battleships)):
        name, coor = battleships[i].split(":")
        coor = translate(coor)

        battleships[i] = (name,coor)
    return battleships

