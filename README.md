# Battlesocket

An online multiplayer implementation of the popular Hasbro strategy game. We are
meant to design and implement an application-layer protocol as well as get
ourselves accustomed to the Unix sockets interface.

## Architecture

> [!WARNING]
>
> Pending.

### Sequence diagram

> [!WARNING]
>
> Pending.

### Class diagram

> [!WARNING]
>
> Pending.

## Protocol

> [!WARNING]
>
> This section is incomplete.

The BattleShip Protocol (BSP) is an application-layer
network communication protocol for Battleship game.
BSP relies on TCP to exchange human-readable text messages
that allow information sharing and event synchronization
between a game server and its clients.

This section defines the semantics of BSP
messages, as well as the procedures regarding them.

### Vocabulary

Following the convention suggested in the instructions handout, the BSP messages
follow the structure:

    message = message_type data "$"

where `$` acts as the message terminator.

Since our game relies on a client-server architecture, some of the messages are
specific to either client or server, and not to its counterpart. Even though, in
theory, a client could send a server-specific message, the protocol procedures
make sure that such cases are handled correctly.

We define some common symbols that are used in the following sections:

    player_letter = "A" | "B"

    coordinate = row col
    row = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J"
    col = "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" | "10"

#### Shared

#### `BAD_REQUEST`

Any malformed request (that is, one that is not a valid message of BSP), is
replied to with a `BAD_REQUEST` message.

    bad_request = "BAD_REQUEST"


#### Server

##### `JOINED_MATCHMAKING`

When a client joins a game server, it is assigned a letter:

    joined_matchmaking = "JOINED_MATCHMAKING" player_letter

##### `START_GAME`

When a game room has been filled, i.e., there are two clients connected, the
server sends a notification to both of them, specifying the start time, who goes
first, and each player's board:

    start_game = "START_GAME" "start_time:"unix_time "initial_player:"letter "board:""{ (ship_type)":""}"

> [!WARNING]
>
> This section is incomplete.

##### `HIT`

When a shot sent by the client results in a hit, the server sends this message
to notify each client of a board update:

    hit = "HIT" coordinate "SUNK" "next:"player_letter

##### `MISS`

Similarly, when a shot sent by the client does _not_ result in a hit, the server
sends this message to notify each client of a board update:

    miss = "MISS" coordinate "next:"player_letter

#### Client

##### `BSP_REQUEST`

> [!WARNING]
>
> Not implemented.


## Project structure

> [!WARNING]
>
> Pending.

## Compilation

### Server

No third-party dependencies.

We are using GNU's `gcc` compiler (specifically, version 11.4.0 on Ubuntu 22.04).

Compile with `make` or compile _and run_ with `make run`.

### Client

Written in Python.

No third-party Python modules.

We are running Python 3.11.1 on Ubuntu 22.04.

You can either run it

1. from the `battlesocket-client` directory with `python src/main.py`

2. or from the project root directory with `python battlesocket-client/src/main.py`


## Authors

> [!WARNING]
>
> Pending.

## References



## License

> [!WARNING]
>
> Pending.
