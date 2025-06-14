<!-- omit from toc -->
# battlesocket
<!-- As HTML tag to prevent it from being included in header numbering. -->

Battlesocket is a client-server architecture implementation of Battleship™, the
popular Hasbro strategy game, that supports the BattleShip Protocol over TCP
using the Unix `sockets` application programming interface.

This README covers the features of both server and client, as well as the
BattleShip Protocol's vocabulary and procedure.

<!-- omit from toc -->
## Contents

- [1. Features](#1-features)
  - [1.1. Server](#11-server)
  - [1.2. Client](#12-client)
- [2. Demo](#2-demo)
- [3. Protocol](#3-protocol)
  - [3.1. Vocabulary](#31-vocabulary)
    - [3.1.1. Shared by client and server](#311-shared-by-client-and-server)
    - [3.1.2. `BAD_REQUEST`](#312-bad_request)
    - [3.1.3. Server-to-client](#313-server-to-client)
      - [3.1.3.1. `JOINED_MATCHMAKING`](#3131-joined_matchmaking)
      - [3.1.3.2. `START_GAME`](#3132-start_game)
      - [3.1.3.3. `END_GAME`](#3133-end_game)
      - [3.1.3.4. `HIT`](#3134-hit)
      - [3.1.3.5. `MISS`](#3135-miss)
      - [3.1.3.6. `TURN`](#3136-turn)
    - [3.1.4. Client-to-server](#314-client-to-server)
      - [3.1.4.1. `JOIN`](#3141-join)
      - [3.1.4.2. `SHOT`](#3142-shot)
      - [3.1.4.3. `SURRENDER`](#3143-surrender)
  - [3.2. Procedure](#32-procedure)
    - [3.2.1. Connection](#321-connection)
    - [3.2.2. Gameplay](#322-gameplay)
    - [3.2.3. End](#323-end)
    - [3.2.4. Edge cases](#324-edge-cases)
  - [3.3. Case examples](#33-case-examples)
    - [3.3.1. Normal development of game](#331-normal-development-of-game)
    - [3.3.2. Bad request](#332-bad-request)
    - [3.3.3. Disconnection](#333-disconnection)
    - [3.3.4. Surrender](#334-surrender)
    - [3.3.5. Timeout](#335-timeout)
    - [3.3.6. No handshake](#336-no-handshake)
    - [3.3.7. Wrong handshake](#337-wrong-handshake)
- [4. Compilation](#4-compilation)
  - [4.1. Server](#41-server)
  - [4.2. Client](#42-client)
- [5. Conclusions](#5-conclusions)
- [6. Authors](#6-authors)
- [7. References](#7-references)
- [8. License](#8-license)


## 1. Features

What's a protocol useful for if there are no programs that implement it?

### 1.1. Server

We wrote a concurrent multi-threaded server in C, using the GNU C library for
common tasks (input/output, string manipulation, etc.) as well as creation and
handling of POSIX threads and input/output multiplexing with `poll()`.

The server accepts incoming connections (after a handshake packet is sent).
It handles matchmaking in a primitive way: using a two-spots FIFO "queue".

After a match starts, a thread is created to handle the game:

- to keep and update its state (the boards of each player, whose turn it is)

- after action messages sent by the players (which are validated by the server
  and logged)

- whose effects are notified to both players (whether it was hit or miss,
  another player disconnected or surrendered, a change of turn, etc.) and logged

- and, when it occurs, tell players who won the match.

In between the match, the server keeps a 30 seconds timer. If the player who has
to shoot doesn't send their shot, it timeouts and the game ends (we preferred this
over continuously switching turns if both players are idle).


### 1.2. Client

The client's remit is:

- To provide a command-line game interface to the player that tells them about:
  - Their own board: Where their ships are and whether they have been hit
  - Their opponent's board: Where they have shot and whether they missed or not
  - Whose turn it is

- To let the player make their shot, surrender, or find a new game.

- To connect to the server using the BSP protocol through message exchanging
  (using the Python Library `sockets` module).

- To detect and notify the player if the client disconnected from the server.


## 2. Demo

Server logging on the bottom, both clients on the top.

> [!IMPORTANT]
>
> 👆 Click on the image below to _download_ the demo (it's a video, but GitHub
> won't show you a video player `¯\_(ツ)_/¯`. Oh, and there's also _no_ embedded
> player🥲)

[![Battlesocket demo preview.](docs/images/battlesocket-demo-preview.png)](docs/videos/battlesocket-demo.mp4)


## 3. Protocol

The BattleShip Protocol (BSP) is an application-layer
network communication protocol for Battleship game.
BSP relies on TCP to exchange human-readable text messages
that allow information sharing and event synchronization
between a game server and its clients.

This section defines the semantics of BSP
messages, as well as the procedures regarding them.

### 3.1. Vocabulary

Following the convention suggested in the instructions handout, the BSP messages
follow the structure:

    message = message_type data LF

where LF acts as the message terminator.

Since our game relies on a client-server architecture, some of the messages are
specific to either client or server, and not to its counterpart. Even though, in
theory, a client could send a server-specific message, the protocol procedures
make sure that such cases are handled correctly.

We define some common symbols that are used in the following sections:

    player_letter = "A" | "B"

    coordinate = row col
    row = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J"
    col = "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" | "10"

#### 3.1.1. Shared by client and server

#### 3.1.2. `BAD_REQUEST`

Any malformed request (that is, one that is not a valid message of BSP), is
replied to with a `BAD_REQUEST` message.

    bad_request = "BAD_REQUEST"


#### 3.1.3. Server-to-client

##### 3.1.3.1. `JOINED_MATCHMAKING`

When a client joins a game server, we send the following message:

    joined_matchmaking = "JOINED_MATCHMAKING"

##### 3.1.3.2. `START_GAME`

When a game room has been filled, i.e., there are two clients connected, the
server notifies both of them that a game will start, along with their respective
letters (`A` or `B`) and boards:

    start_game = "START_GAME" player_letter "{" (ship_type ":" coordinate+)+ "}"
    ship_type = "carrier" | "battleship" | "cruiser" | "destroyer" | "submarine"

##### 3.1.3.3. `END_GAME`

When a game ends the server notifies each client of the reason for ending
the game. These are the possible reasons to end a game:

1. a player won the game

2. a player disconnected from the server

3. a player sent a surrender message to the server

4. a player didn't send a valid action message on time and thus timed out

Thus we have these possible messages:

    end_game = "END_GAME" cause player_letter
    cause    = "WINNER" | "SURRENDER" | "DISCONNECTION" | "TIMEOUT"

##### 3.1.3.4. `HIT`

When a shot sent by the client results in a hit, the server sends this message
to notify each client of a board update:

    hit = "HIT" coordinate | "HIT" coordinate "SUNK"

##### 3.1.3.5. `MISS`

Similarly, when a shot sent by the client does _not_ result in a hit, the server
sends this message to notify each client of a board update:

    miss = "MISS" coordinate

##### 3.1.3.6. `TURN`

When a player shot sent by a client is processed, it tells both players who
goes next through the following message:

    turn = "TURN" player_letter

#### 3.1.4. Client-to-server

##### 3.1.4.1. `JOIN`

A client that wants to connect to a BSP server must send a handshake message to
verify that it's a BSP client and prevent unwanted connections:

    join = "JOIN"

##### 3.1.4.2. `SHOT`

When a player inputs a valid coordinate, the client sends a shot message
with its corresponding values.

    shot = "SHOT" coordinate

##### 3.1.4.3. `SURRENDER`

When a player gives up, the client sends the server a notification with the
following message:

    surrender = "SURRENDER"


### 3.2. Procedure

#### 3.2.1. Connection

1. A BSP client connects to a BSP server over TCP and sends the handshake
   message (`JOIN`) within the time the server was prepared to wait (usually 2
   seconds).

      1. If the handshake message times out, the server closes the TCP
         connection with the client.

      2. After a handshake message is received, the server sends a
         `JOINED_MATCHMAKING` message to the client, with no need for
         acknowledgment.

#### 3.2.2. Gameplay

3. When there's enough players to start a game, the server sends a `START_GAME`
   message to both connected clients with their corresponding letter and game
   board. Each client must have a unique letter (either `A` or `B`) and they
   should only receive their game board, not their opponent's.

4. Shortly after this, the server sends a `TURN` message which notifies both
   clients whose turn it is (either `A` or `B`).

5. The current player must send a `SHOT` message within 30 seconds. Otherwise
   the server will timeout and the server will notify the current opponent that
   they have won the match as the current player timed out.

    1. If the `SHOT` message has valid coordinates, the server notifies both
       clients whether it was a `HIT`, and whether a ship sunk, or `MISS`.

    2. Otherwise, the server sends a `BAD_REQUEST` message to the client. It's
       not essential that the server disconnects the client if it's incorrectly
       implemented: it will timeout eventually.

 #### 3.2.3. End

6. After each `HIT` or `MISS`:

   1. If the game end condition has been met, the server will send an `END_GAME`
      message specifying it ends because of a `WINNER` along with the letter of
      the player who won.

   3. Otherwise, it will change the turn and notify both players by sending a
      `TURN` message.

#### 3.2.4. Edge cases

7. If a client disconnection is detected, the server will notify the other
   player who, presumably, has not disconnected, with an `END_GAME` message that
   specifies `DISCONNECTION` and the letter of the player who disconnected.

8. Similarly, if a client surrenders mid-game, the server will notify the other
   player with an `END_GAME` message that specifies `SURRENDER` and the letter
   of the player who surrendered.


### 3.3. Case examples

To illustrate how the protocol, we showcase a non-exhausting collection of UML
sequence diagrams. These include cases for:

1. The "perfect" game, where communication between clients and server occurs
   without issue.

2. The (mostly) unproblematic game, where a few malformed requests are received,
  amounting to no more than petty errors which can be handled.

3. The fatal-error game, where it's no longer possible to keep playing and it's
   terminated by the server.

4. The handshake procedure, which verifies that the client connecting to the
   server is a BSP client.

#### 3.3.1. Normal development of game

```mermaid
sequenceDiagram
    participant ClientA as Client A
    participant Server as Server
    participant ClientB as Client B

    ClientA->>Server: JOIN
    Server-->>ClientA: JOINED_MATCHMAKING

    ClientB->>Server: JOIN
    Server-->>ClientB: JOINED_MATCHMAKING

    Server-->>ClientA: START_GAME A <board>
    Server-->>ClientB: START_GAME B <board>

    Server-->>ClientA: TURN A
    Server-->>ClientB: TURN A

    ClientA->>Server: SHOT <coordinate>

    Server-->>ClientA: HIT | MISS <coordinate>
    Server-->>ClientB: HIT | MISS <coordinate>

    Server-->>ClientA: TURN B
    Server-->>ClientB: TURN B

    Note over ClientA,ClientB: ... more attempts ...

    ClientA->>Server: SHOT <coordinate>
    Server-->>ClientA: HIT <coordinate> SUNK
    Server-->>ClientB: HIT <coordinate> SUNK

    Server-->>ClientA: END_GAME WINNER A
    Server-->>ClientB: END_GAME WINNER A
```


#### 3.3.2. Bad request

```mermaid
sequenceDiagram
    participant ClientA as Client A
    participant Server as Server
    participant ClientB as Client B

    ClientA->>Server: JOIN
    Server-->>ClientA: JOINED_MATCHMAKING

    ClientB->>Server: JOIN
    Server-->>ClientB: JOINED_MATCHMAKING

    Server-->>ClientA: START_GAME A <board>
    Server-->>ClientB: START_GAME B <board>

    Server-->>ClientA: TURN A
    Server-->>ClientB: TURN A

rect rgb(255,100,76)
    ClientA->>Server: SHOT K11
    Server-->>ClientA: BAD_REQUEST
end

rect rgb(76, 179, 255)
    ClientA->>Server: SHOOT J10
    Server-->>ClientA: BAD_REQUEST
end

    ClientA->>Server: SHOT J10
    Server-->>ClientA: HIT | MISS J10
    Server-->>ClientB: HIT | MISS J10

    Server-->>ClientA: TURN B
    Server-->>ClientB: TURN B

    Note over ClientA,ClientB: ... more attempts ...

    ClientA->>Server: SHOT <coordinate>
    Server-->>ClientA: HIT <coordinate> SUNK
    Server-->>ClientB: HIT <coordinate> SUNK

    Server-->>ClientA: END_GAME WINNER A
    Server-->>ClientB: END_GAME WINNER A
```


#### 3.3.3. Disconnection

```mermaid
sequenceDiagram
    participant ClientA as Client A
    participant Server as Server
    participant ClientB as Client B

    opt Game setup and early turns
      Note over ClientA, ClientB: Players have joined and the game has started
    end

    Server-->>ClientA: TURN B
    Server-->>ClientB: TURN B

    break Client B disconnected
      Server-->>ClientB: "END_GAME DISCONNECTION B"
      Server-->>ClientA: "END_GAME DISCONNECTION B"
    end
```


#### 3.3.4. Surrender

```mermaid
sequenceDiagram
    participant ClientA as Client A
    participant Server as Server
    participant ClientB as Client B

    opt Game setup and early turns
      Note over ClientA, ClientB: Players have joined and the game has started
    end

    Server-->>ClientA: TURN B
    Server-->>ClientB: TURN B

    ClientB->>Server: SURRENDER

    Server-->>ClientB: "END_GAME SURRENDER B"
    Server-->>ClientA: "END_GAME SURRENDER B"
```


#### 3.3.5. Timeout

```mermaid
sequenceDiagram
    participant ClientA as Client A
    participant Server as Server
    participant ClientB as Client B

    opt Game setup and early turns
      Note over ClientA, ClientB: Players have joined and the game has started
    end

    Server-->>ClientA: TURN B
    Server-->>ClientB: TURN B

    Note over ClientB: 30 seconds timeout (no action)

    break End of the game by TIMEOUT
      Server-->>ClientB: "END_GAME TIMEOUT B"
      Server-->>ClientA: "END_GAME TIMEOUT B"
    end
```


#### 3.3.6. No handshake

```mermaid
sequenceDiagram
    participant Client as Client
    participant Server as Server

    Note over Server,Client: Client connected
    Note over Server,Client: awaiting JOIN (5 seconds timeout)

    break Handshake not received
        Server->>Server: Log handshake not received
    end
```


#### 3.3.7. Wrong handshake

```mermaid
sequenceDiagram
    participant Client as Client
    participant Server as Server

    Client->>Server: PLAY

    break Handshake not received
        Server->>Server: Log handshake not received
    end
```


## 4. Compilation

### 4.1. Server

No third-party dependencies.

We are using GNU's `gcc` compiler (specifically, version 11.4.0 on Ubuntu 22.04).

```shell
# Change working directory.
cd battlesocket-server

# Compile.
make -j 4

# Run it with either
make run
# or, to pass *optional* flags, as
./battlesocket-server -p[port] -l[log_file]
```


### 4.2. Client

Written in Python 3 with an embedded third-party.

Tested with Python 3.11.1 on Ubuntu 22.04.

```shell
# Change your working directory to `battlesocket-client`
cd battlesocket-client

# Start Python interpreter on main module.
# *Must* pass IP and port of host.
# (Log path is optional, as it has a default value.)
python src/main.py [server_ip] [server_port] -l [log_path]
```

## 5. Conclusions

We designed an application-layer network communication protocol that works well
to handle a Battleship game. We tried to define it once and for all from the
start, but it took a lot of iterations and nitpicking on details to finally
settle on its design.

During its development, we learned how to use the basic system calls provided by
the Unix `sockets` API. Besides knowing how to interact with this interface, we
had to dig a bit more into the way it's implemented to understand its
limitations and work around them one way or another.

As network applications are typically concurrent, we also learned —despite none
of us having taken a course in operating systems— about threads and,
consequently, about the problem of synchronization which is usually solved
through the use of mutexes or "locks". Trying to understand how mutexes work led
us to learn about atomic operations in computer processors.

In spite of strong suggestion to the contrary, we decided to create a new thread
each time a game started instead of keeping a thread pool. We saw some simple
thread pool implementations but noticed how their higher complexity can carry
along additional problems. These were not worth trying to solve as we had no
real need to handle time-sensitive communications.

## 6. Authors

Jerónimo Acosta Acevedo,
Juan José Restrepo Higuita,
and Luis Miguel Torres Villegas.

## 7. References

1. Hall, Brian. _Beej’s Guide to Network Programming: Using Internet Sockets_.
   2025, https://beej.us/guide/bgnet/.

   Explains how to use the system calls provided by the Unix API for `sockets`
   in a friendly and straightforward way.
   Section 7.2 _`poll()`—Synchronous I/O Multiplexing_ was particularly useful
   in implementing the thread function that handled a game.

2. Blelloch, Guy. _Linux Tutorial: POSIX Threads._ 15-492: Parallel Algorithms
   (Fall 2007),
   https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html.

   Goes over the basic functions provided by the POSIX thread (`pthread`)
   libraries.
   It covers creation, termination, and synchronization using mutexes, among
   other more advanced subjects.

3. Parmaksız, Orhun. _battleship-rs_. 0.1.1, 14 May 2022,
   https://github.com/orhun/battleship-rs.

   Server-side Rust implementation of Battleship. That is, it provides both the
   service _and_ the command-line text interface through a TCP connection (more
   or less like a mainframe interacting with dumb terminals). We designed the
   CLI for our implementation somewhat based off this one.



## 8. License

Copyright 2025 The Authors
