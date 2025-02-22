// serveur_date_time.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 4001
#define BUFFER_SIZE 1024

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Création du socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(1);
    }

    // Configuration du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(1);
    }

    // Écoute
    if (listen(server_sock, 5) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(1);
    }

    printf("Serveur Date-Time écoute sur le port %d...\n", PORT);

    while (1) {
        // Acceptation des connexions
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Gestion de la demande (envoyer la date et l'heure)
        time_t current_time;
        time(&current_time);
        snprintf(buffer, sizeof(buffer), "Date et Heure: %s", ctime(&current_time));

        send(client_sock, buffer, strlen(buffer), 0);
        close(client_sock);
    }

    close(server_sock);
    return 0;
}


