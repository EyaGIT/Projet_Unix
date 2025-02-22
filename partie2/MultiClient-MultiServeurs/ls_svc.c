// ls_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include "struct.h"

#define PORT 4002  // Port doit être 4002 comme défini dans le proxy

void handle_ls_request(int client_sock, msg *message) {
    DIR *dir;
    struct dirent *entry;
    char temp[4096] = "";

    dir = opendir(".");
    if (dir == NULL) {
        strcpy(message->buff, "Erreur: Impossible d'ouvrir le répertoire\n");
    } else {
        while ((entry = readdir(dir)) != NULL) {
            if (strlen(temp) + strlen(entry->d_name) + 2 < sizeof(message->buff)) {
                strcat(temp, entry->d_name);
                strcat(temp, "\n");
            }
        }
        strcpy(message->buff, temp);
        closedir(dir);
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

    // Création du socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(1);
    }

    // Enable address reuse
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
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

    printf("Serveur LS écoute sur le port %d...\n", PORT);

    while (1) {
        // Acceptation des connexions
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Nouvelle connexion acceptée\n");

        // Réception de la requête
        if (recv(client_sock, &message, sizeof(msg), 0) <= 0) {
            perror("recv failed");
            close(client_sock);
            continue;
        }

        // Traitement de la requête
        handle_ls_request(client_sock, &message);
        
        close(client_sock);
    }

    close(server_sock);
    return 0;
}
