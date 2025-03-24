#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include "server.h"
#include "logger.h"
#include "protocol.h"
#include "game.h"


int server_fd;
Client clients[MAX_CLIENTS];
int client_count = 0;
Board boards[MAX_CLIENTS]; // Boards for each player ( ͡~ ͜ʖ ͡°)


void send_to_client(int client_index, const char *message) {
    send(clients[client_index].sockfd, message, strlen(message), 0);
}

// Juan Carlos: "Broadca'ting"
void broadcast(const char *message) {
    for (int i = 0; i < client_count; i++) {
        send_to_client(i, message);
    }
}

void init_server() {

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        log_event("Failed to create socket");
        fprintf(stderr, "[Error] Failed to create socket: %s\n", strerror(errno));
        return;
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr = { htonl(INADDR_ANY) },
    };

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        log_event("SO_REUSEADDR failed");
        fprintf(stderr, "[Error] SO_REUSEADDR failed: %s \n", strerror(errno));
    }

    if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) {
        log_event("Failed to bind");
        fprintf(stderr, "[Error] Failed to bind: %s\n", strerror(errno));
        return;
    }
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        log_event("Failed to listen");
        fprintf(stderr, "[Error] Failed to listen: %s\n", strerror(errno));
        return;
    }
    log_event("Server initialized and listening...");
    printf("Server created with fd %d\n", server_fd);
}

void run_server() {

    while (client_count < MAX_CLIENTS) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len;
        client_addr_len = sizeof(client_addr);

        int new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (new_socket < 0) {
            log_event("Error accepting connection");
            continue;
        }
        clients[client_count].sockfd = new_socket;
        clients[client_count].addr = client_addr;
        clients[client_count].letter = (client_count == 0) ? 'A' : 'B';
        client_count++;
        log_event("New client connected");
        // Send JOINED_MATCHMAKING
        char buffer[256];
        build_joined_matchmaking(buffer, clients[client_count - 1].letter);
        send_to_client(client_count - 1, buffer);
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        init_board(&boards[i]);
        place_ships(&boards[i]);
    }

    long start_time = time(NULL) + 5;
    srand(time(NULL));
    int current_player = rand() % 2;
    char initial_player[2];
    initial_player[0] = (current_player == 0) ? 'A' : 'B';
    initial_player[1] = '\0';
    
    // Send to each client the START_GAME with their boards
    for (int i = 0; i < MAX_CLIENTS; i++) {
        char ship_data[1024] = "";
        get_ship_data(&boards[i], ship_data, sizeof(ship_data));
        char start_msg[2048] = "";
        // "START_GAME|<unix_time> <initial_player> <ship_data>\n"
        build_start_game(start_msg, start_time, initial_player, ship_data);
        send_to_client(i, start_msg);
        log_event(start_msg);
    }

    log_event("Game started...");
    
    int game_over = 0;
    char recv_buffer[256];
    char action_msg[256];
    while (!game_over) {
        memset(recv_buffer, 0, sizeof(recv_buffer));
        int bytes_read = recv(clients[current_player].sockfd, recv_buffer, sizeof(recv_buffer) - 1, 0);
        if (bytes_read <= 0) {
            log_event("Failed recieving data or client disconnection");
            break;
        }
        int newline_pos = strcspn(recv_buffer, "\r\n");
        recv_buffer[newline_pos] = '\0';

        log_event("Player message recieved");
        if (parse_message(recv_buffer) != MSG_SHOT) {
            send_to_client(current_player, "BAD_REQUEST|\n");
            continue;
        }
        
        // We expect a message like "SHOT|A-1"
        char *separator = strchr(recv_buffer, '|');
        if (separator == NULL) {
            send_to_client(current_player, "BAD_REQUEST|\n");
            continue;
        }
        char pos[16] = {0};
        strncpy(pos, separator + 1, sizeof(pos) - 1);
        

        if (pos[0] < 'A' || pos[0] > 'J' || pos[1] != '-') {
            send_to_client(current_player, "BAD_REQUEST|\n");
            continue;
        }
        char row_char = pos[0];
        int row = row_char - 'A';
        int col = atoi(pos + 2) - 1;

        log_event("Processing shot...");

        // The shot happens in the board of the opposing player
        int hit = validate_shot(&boards[1 - current_player], row, col);
        update_board(&boards[1 - current_player], row, col, hit);

        int sunk = 0;
        if (hit) {
            int ship_index = get_ship_index_at(&boards[1 - current_player], row, col);
            if (ship_index != -1)
                sunk = is_ship_sunk(&boards[1 - current_player], ship_index);
        }

        const char *result = hit ? "HIT" : "MISS";
        char next_player = current_player == 0 ? 'B' : 'A';

        memset(action_msg, 0, sizeof(action_msg));
        build_action_result(action_msg, result, pos, sunk, (char[]){next_player, '\0'});
        broadcast(action_msg);
        log_event("Action message sent");

        // Check if the other player still stands (˘･_･˘)
        if (is_game_over(&boards[1 - current_player])) {
            char end_msg[256];
            build_end_game(end_msg, (char[]){current_player == 0 ? 'A' : 'B', '\0'});
            broadcast(end_msg);
            game_over = 1;
            log_event("Game over");
            break;
        }
        // Player turn change
        current_player = 1 - current_player;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        close(clients[i].sockfd);
    }
}

void cleanup_server() {
    close(server_fd);
    log_event("Server closed");
}