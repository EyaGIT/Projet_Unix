#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include "struct.h"

#define PORT 4004

// Fonction pour calculer et envoyer la durée de connexion
void send_connection_duration(int client_sock, msg *message) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    
    // Calcul de la durée en secondes et nanosecondes
    double duration = (now.tv_sec - message->client_connection_time.tv_sec) + 
                     (now.tv_nsec - message->client_connection_time.tv_nsec) / 1e9;
    
    // Prépare le message avec la durée
    snprintf(message->buff, sizeof(message->buff), 
             "Durée de connexion: %.2f secondes\n", duration);

    // Envoi du message avec la durée de connexion
    if (send(client_sock, message, sizeof(msg), 0) < 0) {
        perror("send failed (durée de connexion)");
        return;
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

    printf("Serveur Connection Duration écoute sur le port %d...\n", PORT);

    // Accepte plusieurs clients
    while (1) {
        // Acceptation des connexions
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Connexion acceptée\n");

        // Réception du message du client qui contient le timestamp initial
        if (recv(client_sock, &message, sizeof(msg), 0) <= 0) {
            perror("recv failed");
            close(client_sock);
            continue;
        }

        // Envoyer la durée de connexion
        send_connection_duration(client_sock, &message);
        
        close(client_sock);  // Ferme la connexion après avoir répondu
    }

    close(server_sock);  // Fermer le socket du serveur après avoir quitté la boucle
    return 0;
}

