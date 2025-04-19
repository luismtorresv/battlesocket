import argparse
import logging
import sys
from ipaddress import ip_address

from client import Client


def main():
    # Parsing command-line arguments.
    parser = argparse.ArgumentParser(description="client program for battlesocket")
    parser.add_argument("ip", help="the server's public IP (v4)")
    parser.add_argument("port", help="port where game server is listening", type=int)
    parser.add_argument(
        "-l",
        dest="log_path",
        help="path where log file will be written to",
        default="client.log",
        required=False,
    )
    args = parser.parse_args()

    try:
        ip = str(ip_address(args.ip))
    except ValueError:
        print(
            "Not a valid IP address: %s",
            args.ip,
            file=sys.stderr,
        )
        sys.exit(1)

    if args.port > 65535 or args.port < 1:
        print(
            "Not a valid port number: %s",
            args.port,
            file=sys.stderr,
        )
        sys.exit(1)
    else:
        port = args.port

    logging.basicConfig(
        filename=args.log_path,
        filemode="a",
        level=logging.DEBUG,
        format="%(asctime)s - %(levelname)s - %(message)s",
    )

    logging.info("Starting program.")
    print("Welcome to Battlesocket")
    client = Client(ip, port)
    client.run()
    client.cleanup()
    logging.info("Exiting program.")


if __name__ == "__main__":
    main()
