import argparse
import logging
import sys
from ipaddress import ip_address

from client import Client

logging.basicConfig(
    filename="client.log",
    filemode="a",
    level=logging.DEBUG,
    format="%(asctime)s - %(levelname)s - %(message)s",
    datefmt="%Y-%m-%d %I:%M:%S",
)


def main():
    # Parsing command-line arguments.
    parser = argparse.ArgumentParser(
        description="client program for battlesocket",
    )
    parser.add_argument("ip", help="the server's public IP (v4)")
    parser.add_argument("port", help="port where game server is listening", type=int)
    args = parser.parse_args()

    try:
        ip = str(ip_address(args.ip))
    except ValueError:
        logging.error("Not a valid IP address: %s", args.ip)
        sys.exit(1)

    if args.port > 65535 or args.port < 1:
        logging.error("Not a valid port number: %s", args.port)
        sys.exit(1)
    else:
        port = args.port

    logging.info("Starting program.")
    print("Welcome to Battlesocket!")
    client = Client(ip, port)
    try:
        client.username = input("Username: ")
        client.email = input("Email: ")
    except KeyboardInterrupt:
        print()
        sys.exit(1)

    client.run()
    client.cleanup()
    logging.info("Exiting program.")


if __name__ == "__main__":
    main()
