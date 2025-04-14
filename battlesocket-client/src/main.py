import argparse
import sys
from ipaddress import ip_address

from client import Client


def main(args):
    try:
        ip = args.ip
        port = args.port
    except AttributeError:
        print(f"error: could not unpack args: {args}", file=sys.stderr)
        sys.exit(1)

    try:
        client = Client(ip, port)
        print("Welcome to Battlesocket!")
        client.username = input("Username: ")
        client.email = input("Email: ")
    except KeyboardInterrupt:
        print()
        sys.exit(1)

    client.run()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="client program for battlesocket",
    )
    parser.add_argument("ip", help="the server's public IP (v4)")
    parser.add_argument("port", help="port where game server is listening", type=int)
    args = parser.parse_args()

    try:
        ip = str(ip_address(args.ip))
    except ValueError:
        print(f"error: not a valid IP address: {args.ip}", file=sys.stderr)
        sys.exit(1)

    if args.port > 65535 or args.port < 1:
        print(f"error: not a valid port number: {args.port}", file=sys.stderr)
        sys.exit(1)

    main(args)
