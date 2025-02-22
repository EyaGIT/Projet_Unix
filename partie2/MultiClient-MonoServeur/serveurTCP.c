#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

// Fonction pour envoyer le menu des services
void send_menu(int client_sock) {
    const char *menu = "1. Date et heure\n2. Liste des fichiers\n3. Contenu d'un fichier\n4. Durée de connexion\n0. Quitter\n";
    size_t menu_len = strlen(menu);

    // Envoyer la taille du menu
    if (send(client_sock, &menu_len, sizeof(menu_len), 0) < 0) {
        perror("send failed (taille du menu)");
        return;
    }

    // Envoyer le menu
    if (send(client_sock, menu, menu_len, 0) < 0) {
        perror("send failed (menu)");
        return;
    }
}

// Fonction pour envoyer la date et l'heure
void send_date_time(int client_sock) {
    time_t now = time(NULL);
    char *date_time = ctime(&now);
    size_t date_time_len = strlen(date_time);

    // Envoyer la taille de la date et de l'heure
    if (send(client_sock, &date_time_len, sizeof(date_time_len), 0) < 0) {
        perror("send failed (taille de la date et de l'heure)");
        return;
    }

    // Envoyer la date et l'heure
    if (send(client_sock, date_time, date_time_len, 0) < 0) {
        perror("send failed (date et heure)");
        return;
    }
}

// Fonction pour envoyer la liste des fichiers
void send_file_list(int client_sock) {
    DIR *dir;
    struct dirent *entry;
    char buffer[BUFFER_SIZE] = "";

    dir = opendir(".");
    if (dir == NULL) {
        strcpy(buffer, "Erreur: Impossible d'ouvrir le répertoire.\n");
        size_t buffer_len = strlen(buffer);

        // Envoyer la taille du message d'erreur
        if (send(client_sock, &buffer_len, sizeof(buffer_len), 0) < 0) {
            perror("send failed (taille du message d'erreur)");
            return;
        }

        // Envoyer le message d'erreur
        if (send(client_sock, buffer, buffer_len, 0) < 0) {
            perror("send failed (message d'erreur)");
            return;
        }
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        strcat(buffer, entry->d_name);
        strcat(buffer, "\n");
    }
    closedir(dir);

    size_t buffer_len = strlen(buffer);

    // Envoyer la taille de la liste des fichiers
    if (send(client_sock, &buffer_len, sizeof(buffer_len), 0) < 0) {
        perror("send failed (taille de la liste des fichiers)");
        return;
    }

    // Envoyer la liste des fichiers
    if (send(client_sock, buffer, buffer_len, 0) < 0) {
        perror("send failed (liste des fichiers)");
        return;
    }
}

// Fonction pour envoyer le contenu d'un fichier
void send_file_content(int client_sock, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // Si le fichier est introuvable, envoyer un message d'erreur au client
        const char *error_msg = "Erreur: Fichier introuvable.\n";
        size_t error_msg_len = strlen(error_msg);

        // Envoyer la taille du message d'erreur
        if (send(client_sock, &error_msg_len, sizeof(error_msg_len), 0) < 0) {
            perror("send failed (taille du message d'erreur)");
            return;
        }

        // Envoyer le message d'erreur
        if (send(client_sock, error_msg, error_msg_len, 0) < 0) {
            perror("send failed (message d'erreur)");
            return;
        }
        return;
    }

    // Déterminer la taille du fichier
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Envoyer la taille du fichier au client
    if (send(client_sock, &file_size, sizeof(file_size), 0) < 0) {
        perror("send failed (taille du fichier)");
        fclose(file);
        return;
    }

    // Lire le fichier en une seule fois
    char *file_buffer = malloc(file_size);
    if (file_buffer == NULL) {
        perror("malloc failed");
        fclose(file);
        return;
    }

    fread(file_buffer, 1, file_size, file);
    fclose(file);

    // Envoyer le contenu du fichier au client
    if (send(client_sock, file_buffer, file_size, 0) < 0) {
        perror("send failed (contenu du fichier)");
        free(file_buffer);
        return;
    }

    free(file_buffer);
}

// Fonction pour envoyer la durée de connexion
void send_connection_duration(int client_sock, time_t start_time) {
    time_t now = time(NULL);
    double duration = difftime(now, start_time);
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "Durée de connexion: %.2f secondes\n", duration);
    size_t buffer_len = strlen(buffer);

    // Envoyer la taille de la durée de connexion
    if (send(client_sock, &buffer_len, sizeof(buffer_len), 0) < 0) {
        perror("send failed (taille de la durée de connexion)");
        return;
    }

    // Envoyer la durée de connexion
    if (send(client_sock, buffer, buffer_len, 0) < 0) {
        perror("send failed (durée de connexion)");
        return;
    }
}

// Fonction pour gérer un client
void handle_client(int client_sock) {
    char username[BUFFER_SIZE], password[BUFFER_SIZE];
    time_t start_time = time(NULL);

    // Initialiser les buffers
    memset(username, 0, BUFFER_SIZE);
    memset(password, 0, BUFFER_SIZE);

    // Recevoir la taille du nom d'utilisateur
    size_t username_len;
    if (recv(client_sock, &username_len, sizeof(username_len), 0) < 0) {
        perror("recv failed (taille du nom d'utilisateur)");
        close(client_sock);
        return;
    }

    // Recevoir le nom d'utilisateur
    if (recv(client_sock, username, username_len, 0) < 0) {
        perror("recv failed (nom d'utilisateur)");
        close(client_sock);
        return;
    }

    // Recevoir la taille du mot de passe
    size_t password_len;
    if (recv(client_sock, &password_len, sizeof(password_len), 0) < 0) {
        perror("recv failed (taille du mot de passe)");
        close(client_sock);
        return;
    }

    // Recevoir le mot de passe
    if (recv(client_sock, password, password_len, 0) < 0) {
        perror("recv failed (mot de passe)");
        close(client_sock);
        return;
    }

    // Vérifier les identifiants
    if (strcmp(username, "user") == 0 && strcmp(password, "pass") == 0) {
        const char *success_msg = "Authentification réussie.\n";
        size_t success_msg_len = strlen(success_msg);

        // Envoyer la taille du message de succès
        if (send(client_sock, &success_msg_len, sizeof(success_msg_len), 0) < 0) {
            perror("send failed (taille du message de succès)");
            close(client_sock);
            return;
        }

        // Envoyer le message de succès
        if (send(client_sock, success_msg, success_msg_len, 0) < 0) {
            perror("send failed (message de succès)");
            close(client_sock);
            return;
        }

        // Envoyer le menu des services
        printf("Envoi du menu initial...\n");
        send_menu(client_sock);
    } else {
        const char *fail_msg = "Échec de l'authentification.\n";
        size_t fail_msg_len = strlen(fail_msg);

        // Envoyer la taille du message d'échec
        if (send(client_sock, &fail_msg_len, sizeof(fail_msg_len), 0) < 0) {
            perror("send failed (taille du message d'échec)");
            close(client_sock);
            return;
        }

        // Envoyer le message d'échec
        if (send(client_sock, fail_msg, fail_msg_len, 0) < 0) {
            perror("send failed (message d'échec)");
            close(client_sock);
            return;
        }

        close(client_sock);
        return;
    }

    // Boucle principale pour gérer les demandes du client
    while (1) {
        int choice;
        if (recv(client_sock, &choice, sizeof(choice), 0) < 0) {
            perror("recv failed (choix du service)");
            break;
        }

        if (choice == 0) {
            printf("Client déconnecté.\n");
            break;
        }

        switch (choice) {
            case 1:
                send_date_time(client_sock);
                break;
            case 2:
                send_file_list(client_sock);
                break;
            case 3:
                {
                    char filename[BUFFER_SIZE];
                    memset(filename, 0, BUFFER_SIZE);

                    // Recevoir la taille du nom du fichier
                    size_t filename_len;
                    if (recv(client_sock, &filename_len, sizeof(filename_len), 0) < 0) {
                        perror("recv failed (taille du nom du fichier)");
                        break;
                    }

                    // Recevoir le nom du fichier
                    if (recv(client_sock, filename, filename_len, 0) < 0) {
                        perror("recv failed (nom du fichier)");
                        break;
                    }

                    // Envoyer le contenu du fichier
                    send_file_content(client_sock, filename);
                }
                break;
            case 4:
                send_connection_duration(client_sock, start_time);
                break;
            default:
                {
                    const char *invalid_msg = "Choix invalide.\n";
                    size_t invalid_msg_len = strlen(invalid_msg);

                    // Envoyer la taille du message d'erreur
                    if (send(client_sock, &invalid_msg_len, sizeof(invalid_msg_len), 0) < 0) {
                        perror("send failed (taille du message d'erreur)");
                        break;
                    }

                    // Envoyer le message d'erreur
                    if (send(client_sock, invalid_msg, invalid_msg_len, 0) < 0) {
                        perror("send failed (message d'erreur)");
                        break;
                    }
                }
                break;
        }

        // Renvoyer le menu après chaque service
        printf("Envoi du menu après le service %d...\n", choice);
        send_menu(client_sock);
    }

    close(client_sock);
}

// Fonction pour ignorer le signal SIGCHLD
void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <server_port>\n", argv[0]);
        exit(1);
    }

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Ignorer le signal SIGCHLD pour éviter les processus zombies
    signal(SIGCHLD, sigchld_handler);

    // Création de la socket TCP
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(1);
    }

    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Liaison de la socket à l'adresse du serveur
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_sock);
        exit(1);
    }

    // Mise en écoute de la socket
    if (listen(server_sock, 5) < 0) {
        perror("listen failed");
        close(server_sock);
        exit(1);
    }

    printf("Serveur TCP en attente de connexions...\n");

    // Accepter les connexions des clients
    while (1) {
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("accept failed");
            continue;
        }

        printf("Nouveau client connecté.\n");

        // Gérer le client dans un nouveau processus
        if (fork() == 0) {
            close(server_sock);  // Fermer la socket du serveur dans le processus enfant
            handle_client(client_sock);
            exit(0);  // Terminer le processus enfant après avoir géré le client
        } else {
            close(client_sock);  // Fermer la socket du client dans le processus parent
        }
    }

    close(server_sock);
    return 0;
}
