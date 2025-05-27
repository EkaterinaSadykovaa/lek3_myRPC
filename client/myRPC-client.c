#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pwd.h>
#include <mysyslog.h>

#define MAX_BUFFER 1024

void print_help() {
    printf("Usage: myRPC-client [options]\n");
    printf("Options:\n");
    printf("  -c, --command \"команда bash\"  Command to execute\n");
    printf("  -h, --host \"ip_addr\"           Server address\n");
    printf("  -p, --port 1234                 Server port\n");
    printf("  -s, --stream                     Use stream socket\n");
    printf("  -d, --dgram                      Use datagram socket\n");
    printf("  --help                            Show this help message\n");
}

void escape_command(char *command) {
    char escaped[MAX_BUFFER];
    int j = 0;
    for (int i = 0; command[i] != '\0'; i++) {
        if (command[i] == '"') {
            escaped[j++] = '\\'; 
        }
        if (command[i] == ';' || command[i] == '|' || command[i] == '`') {
            escaped[j++] = '\\';
        }
        escaped[j++] = command[i];
    }
    escaped[j] = '\0';
    strcpy(command, escaped);
}
int main(int argc, char *argv[]) {
    char *command = NULL;
    char *host = NULL;
    int port = 0;
    int socket_type = 0;
    
    log_json("Клиент запущен");

    int opt;
    while ((opt = getopt(argc, argv, "c:h:p:sd")) != -1) {
        switch (opt) {
            case 'c':
                command = optarg;
                break;
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                socket_type = 1; //stream
                break;
            case 'd':
                socket_type = 2; //datagram
                break;
            case '?':
                print_help();
                exit(EXIT_FAILURE);
        }
    }

   
    if (!command || !host || port <= 0) {
        fprintf(stderr, "Ошибка: Отсутствуют необходимые параметры\n");
        print_help();
        exit(EXIT_FAILURE);
    }

    struct passwd *pw = getpwuid(getuid());
    char *username = pw->pw_name;

    escape_command(command);

    
    int sock = (socket_type == 1) ? socket(AF_INET, SOCK_STREAM, 0) : socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        log_json("Ошибка создания сокета");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        perror("Неверный адрес/ Адрес, который не поддерживается");
        close(sock);
        return EXIT_FAILURE;
    }

    if (socket_type == 1 && connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Не удалось подключиться к серверу");
        close(sock);
        return EXIT_FAILURE;
    }

    char json_request[MAX_BUFFER];
    snprintf(json_request, sizeof(json_request), "{\"login\":\"%s\",\"command\":\"%s\"}", username, command);
    
    if (send(sock, json_request, strlen(json_request), 0) < 0) {
        perror("Не удалось отправить команду");
        close(sock);
        return EXIT_FAILURE;
    }
    
    log_json("Команда отправлена на сервер");

    char buffer[MAX_BUFFER];
    int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        perror("Не удалось получить ответ");
        close(sock);
        return EXIT_FAILURE;
    }

    buffer[bytes_received] = '\0';
    printf("Ответ от сервера: %s\n", buffer);
    close(sock);
    return EXIT_SUCCESS;
}
