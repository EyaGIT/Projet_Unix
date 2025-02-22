// proxy.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include "struct.h"

#define AUTH_PORT 4000
#define DATE_PORT 4001
#define LS_PORT 4002
#define CAT_PORT 4003
#define DUREE_PORT 4004
#define FILES_PORT 4005
#define VALID_USERNAME "user"
#define VALID_PASSWORD "pass"

void LOG_INFO(const char *msg) {
    printf("[INFO] %s\n", msg);
}

int safe_socket() {
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }
    return sock;
}

int safe_connect(int sock, struct sockaddr_in *addr) {
    if (connect(sock, (struct sockaddr *) addr, sizeof(*addr)) < 0) {
        perror("connect failed");
        return -1;
    }
    return 0;
}

// Fonction route_connection mise à jour dans proxy.c
int route_connection(int op) {
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    switch (op) {
        case 1:
            server_addr.sin_port = htons(DATE_PORT);
            LOG_INFO("Routing to Date server");
            break;
        case 2:
            server_addr.sin_port = htons(LS_PORT);  // 4002
            LOG_INFO("Routing to LS server");
            break;
        case 3:
            server_addr.sin_port = htons(CAT_PORT);
            LOG_INFO("Routing to cat server");
            break;
        case 4:
            server_addr.sin_port = htons(DUREE_PORT);
            LOG_INFO("Routing to duree server");
            break;
        default:
            return -1;
    }

    int sock = safe_socket();
    if (sock < 0) {
        LOG_INFO("Failed to create socket");
        return -1;
    }
    
    if (safe_connect(sock, &server_addr) < 0) {
        LOG_INFO("Failed to connect to service");
        close(sock);
        return -1;
    }
    
    return sock;
}

int authenticate_client(int client_sock, msg *message) {
    char username[100], password[100];
    
    if (recv(client_sock, message, sizeof(msg), 0) <= 0)
        return 0;
        
    if (sscanf(message->buff, "%[^:]:%s", username, password) != 2)
        return 0;

    return (strcmp(username, VALID_USERNAME) == 0 && 
            strcmp(password, VALID_PASSWORD) == 0);
}

void handle_client(int client_sock) {
    msg message;
    static int client_counter = 0;
    
    message.client_id = ++client_counter;

    if (!authenticate_client(client_sock, &message)) {
        strcpy(message.buff, "Échec de l'authentification\n");
        send(client_sock, &message, sizeof(msg), 0);
        return;
    }

    strcpy(message.buff, "Authentification réussie\n");
    send(client_sock, &message, sizeof(msg), 0);

    while (1) {
        ssize_t recv_size;
        if ((recv_size = recv(client_sock, &message, sizeof(msg), 0)) <= 0) {
            if (recv_size == 0) {
                LOG_INFO("Client disconnected normally");
            } else {
                perror("recv failed");
            }
            break;
        }

        if (message.op == 5) {
            strcpy(message.buff, "Déconnexion...");
            send(client_sock, &message, sizeof(msg), 0);
            break;
        }

        if (message.op < 1 || message.op > 6) {
            strcpy(message.buff, "Service invalide\n");
            send(client_sock, &message, sizeof(msg), 0);
            continue;
        }

        int server_sock = route_connection(message.op);
        
        if (server_sock == -1) {
            snprintf(message.buff, sizeof(message.buff), 
                    "Erreur de connexion au serveur (service %d)\n", message.op);
            send(client_sock, &message, sizeof(msg), 0);
            continue;
        }

        if (send(server_sock, &message, sizeof(msg), 0) <= 0) {
            strcpy(message.buff, "Erreur d'envoi au serveur\n");
            send(client_sock, &message, sizeof(msg), 0);
            close(server_sock);
            continue;
        }

        if (recv(server_sock, &message, sizeof(msg), 0) <= 0) {
            strcpy(message.buff, "Erreur de réception du serveur\n");
            send(client_sock, &message, sizeof(msg), 0);
            close(server_sock);
            continue;
        }

        send(client_sock, &message, sizeof(msg), 0);
        close(server_sock);
    }
}

void *client_handler(void *arg) {
    int client_sock = *(int*)arg;
    handle_client(client_sock);
    close(client_sock);  // Ensure socket is closed
    free(arg);
    return NULL;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(1);
    }

    // Enable address reuse
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_sock);
        exit(1);
    }
    
    if (listen(server_sock, 5) < 0) {
        perror("listen failed");
        close(server_sock);
        exit(1);
    }

    printf("Serveur Proxy en écoute sur le port %d...\n", 8888);

    while (1) {
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("accept failed");
            continue;
        }

        int *sock_ptr = malloc(sizeof(int));
        if (sock_ptr == NULL) {
            perror("Failed to allocate memory");
            close(client_sock);
            continue;
        }
        *sock_ptr = client_sock;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_handler, (void*)sock_ptr) != 0) {
            perror("Failed to create thread");
            free(sock_ptr);
            close(client_sock);
            continue;
        }
        pthread_detach(thread_id);
    }

    close(server_sock);
    return 0;
}
