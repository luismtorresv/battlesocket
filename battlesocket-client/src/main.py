import sys

from client import Client

if __name__ == "__main__":
    try:
        client = Client()
        print("Welcome to Battlesocket!")
        client.username = input("Username: ")
        client.email = input("Email: ")
    except:
        print()
        sys.exit(1)

    client.run()
