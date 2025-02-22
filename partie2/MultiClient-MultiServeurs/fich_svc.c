// cat_server.c - Serveur corrigé pour lire les fichiers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "struct.h"

#define PORT 4003

void handle_cat_file(int client_sock, msg *message) {
    FILE *file = fopen(message->buff, "r");
    if (!file) {
        snprintf(message->buff, sizeof(message->buff), "Erreur: Impossible d'ouvrir le fichier %s\n", message->buff);
    } else {
        char temp[4096] = "";
        char line[1024];
        while (fgets(line, sizeof(line), file)) {
            if (strlen(temp) + strlen(line) < sizeof(message->buff)) {
                strcat(temp, line);
            }
        }
        strcpy(message->buff, temp);
        fclose(file);
    }

    if (send(client_sock, message, sizeof(msg), 0) < 0) {
        perror("send failed");
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    msg message;

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    if (listen(server_sock, 5) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Serveur Cat écoute sur le port %d...\n", PORT);

    while (1) {
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        if (recv(client_sock, &message, sizeof(msg), 0) <= 0) {
            close(client_sock);
            continue;
        }

        handle_cat_file(client_sock, &message);
        close(client_sock);
    }

    close(server_sock);
    return 0;
}
