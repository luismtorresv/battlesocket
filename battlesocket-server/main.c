#include "errno.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#include "netinet/in.h"
#include "sys/socket.h"

// TODO: Don't hard-code this!!!!!!!!!!!!!!!!!
#define SERVER_PORT (80)

int
main ()
{
    int server_fd;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        fprintf(stderr, "[Error] Failed to create socket: %s\n", strerror(errno));
        return 1;
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr = { htonl(INADDR_ANY) },
    };

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
            0) {
        fprintf(stderr, "[Error] SO_REUSEADDR failed: %s \n", strerror(errno));
    }

    if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0)
    {
        fprintf(stderr, "[Error] Failed to bind: %s\n", strerror(errno));
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0)
    {
        fprintf(stderr, "[Error] Failed to listen: %s\n", strerror(errno));
        return 1;
    }

    printf("Waiting for a client to connect...\n");
    printf("Server created with fd %d\n", server_fd);

    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    client_addr_len = sizeof(client_addr);

    while (true)
    {
        int consocket = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        printf("Client connected\n");

        char buffer[128 + 1] = { 0 };
        recv(consocket, buffer, 128 + 1, 0);
        printf("rcv'd: %s\n", buffer);

        char *response = "HELLO WORLD";
        ssize_t status = send(consocket, response, strlen(response), 0);
        printf("Sent response.\n");

        shutdown(consocket, 0);
    }
}
