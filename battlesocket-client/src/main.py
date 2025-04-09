import sys

from client import Client

if __name__ == "__main__":
    client = Client()

    print("Welcome to Battlesocket!")
    try:
        client.username = input("Username: ")
        client.email = input("Email: ")
    except EOFError:  # The user should not send ^D, but oh well.
        print()
        sys.exit(1)

    client.run()
