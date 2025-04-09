PORT = 8080

BOARD_SIZE = 10

VOCAB = {
    "A": 1,
    "B": 2,
    "C": 3,
    "D": 4,
    "E": 5,
    "F": 6,
    "G": 7,
    "H": 8,
    "I": 9,
    "J": 10,
}

# Code inspired by:
# https://stackoverflow.com/questions/55486225/check-if-string-has-a-certain-format
# The expected input is a letter from A-J concatenated with a number from 1-10.
EXPECTED_INPUT = r"^[A-J]([1-9]|10)$"
