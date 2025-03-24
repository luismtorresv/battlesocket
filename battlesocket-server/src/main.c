#include "server.h"
#include "logger.h"

int main() {
    init_logger();
    init_server();
    run_server();
    cleanup_server();
    close_logger();
    return 0;
}