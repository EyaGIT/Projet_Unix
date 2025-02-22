#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void authenticate(int sockfd) {
    char username[BUFFER_SIZE], password[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Initialiser les buffers
    memset(username, 0, BUFFER_SIZE);
    memset(password, 0, BUFFER_SIZE);
    memset(response, 0, BUFFER_SIZE);

    // Demander les informations d'authentification
    printf("Nom d'utilisateur: ");
    scanf("%s", username);
    printf("Mot de passe: ");
    scanf("%s", password);

    // Envoyer la taille du nom d'utilisateur
    size_t username_len = strlen(username);
    send(sockfd, &username_len, sizeof(username_len), 0);

    // Envoyer le nom d'utilisateur
    send(sockfd, username, username_len, 0);

    // Envoyer la taille du mot de passe
    size_t password_len = strlen(password);
    send(sockfd, &password_len, sizeof(password_len), 0);

    // Envoyer le mot de passe
    send(sockfd, password, password_len, 0);

    // Recevoir la réponse du serveur
    size_t response_len;
    if (recv(sockfd, &response_len, sizeof(response_len), 0) < 0) {
        perror("recv failed");
        exit(1);
    }

    if (recv(sockfd, response, response_len, 0) < 0) {
        perror("recv failed");
        exit(1);
    }

    // Afficher la réponse du serveur
    printf("%s\n", response);
}

void receive_file_content(int sockfd) {
    // Recevoir la taille du fichier
    long file_size;
    if (recv(sockfd, &file_size, sizeof(file_size), 0) < 0) {
        perror("recv failed (taille du fichier)");
        return;
    }

    // Allouer un buffer pour contenir le fichier
    char *file_buffer = malloc(file_size);
    if (file_buffer == NULL) {
        perror("malloc failed");
        return;
    }

    // Recevoir le contenu du fichier
    size_t total_received = 0;
    while (total_received < file_size) {
        ssize_t received = recv(sockfd, file_buffer + total_received, file_size - total_received, 0);
        if (received < 0) {
            perror("recv failed (contenu du fichier)");
            free(file_buffer);
            return;
        }
        total_received += received;
    }

    // Afficher le contenu du fichier
    printf("Contenu du fichier :\n%.*s\n", (int)file_size, file_buffer);

    free(file_buffer);
}

void display_services(int sockfd) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Recevoir la taille du menu
    size_t menu_len;
    if (recv(sockfd, &menu_len, sizeof(menu_len), 0) < 0) {
        perror("recv failed (taille du menu)");
        exit(1);
    }

    // Recevoir le menu
    if (recv(sockfd, buffer, menu_len, 0) < 0) {
        perror("recv failed (menu)");
        exit(1);
    }

    // Afficher le menu
    printf("%s\n", buffer);
}

void request_service(int sockfd) {
    while (1) {
        int choice;
        printf("Choisissez un service (1-4) ou 0 pour quitter: ");
        scanf("%d", &choice);

        // Envoyer le choix au serveur
        if (send(sockfd, &choice, sizeof(choice), 0) < 0) {
            perror("send failed");
            exit(1);
        }

        if (choice == 0) {
            printf("Déconnexion...\n");
            break;  // Quitter la boucle
        }

        // Si le choix est 3, demander le nom du fichier
        if (choice == 3) {
            char filename[BUFFER_SIZE];
            printf("Entrez le nom du fichier: ");
            scanf("%s", filename);

            // Envoyer la taille du nom du fichier
            size_t filename_len = strlen(filename);
            if (send(sockfd, &filename_len, sizeof(filename_len), 0) < 0) {
                perror("send failed (taille du nom du fichier)");
                break;
            }

            // Envoyer le nom du fichier
            if (send(sockfd, filename, filename_len, 0) < 0) {
                perror("send failed (nom du fichier)");
                break;
            }

            // Recevoir le contenu du fichier
            receive_file_content(sockfd);
        } else {
            // Recevoir la taille de la réponse du serveur
            size_t response_len;
            if (recv(sockfd, &response_len, sizeof(response_len), 0) < 0) {
                perror("recv failed (taille de la réponse)");
                exit(1);
            }

            // Recevoir la réponse du serveur
            char response[BUFFER_SIZE];
            memset(response, 0, BUFFER_SIZE);
            if (recv(sockfd, response, response_len, 0) < 0) {
                perror("recv failed (réponse du serveur)");
                exit(1);
            }

            // Afficher la réponse du serveur
            printf("Réponse du serveur: %s\n", response);
        }

        // Recevoir et afficher le menu après chaque service
        display_services(sockfd);
    }

    // Fermer la socket du client
    close(sockfd);
    printf("Client déconnecté. Au revoir !\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in server_addr;

    // Création de la socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(1);
    }

    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Connexion au serveur
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connection failed");
        close(sockfd);
        exit(1);
    }

    // Authentification
    authenticate(sockfd);

    // Boucle principale pour interagir avec le serveur
    while (1) {
        display_services(sockfd);
        request_service(sockfd);
    }

    close(sockfd);
    return 0;
}
