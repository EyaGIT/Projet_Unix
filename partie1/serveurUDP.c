#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define NMAX 100  // Valeur maximale pour n

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <server_port>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    int n;
    socklen_t client_addr_len = sizeof(client_addr);

    // Création de la socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(1);
    }

    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));  // Port du serveur
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Écoute sur toutes les interfaces

    // Liaison de la socket à l'adresse du serveur
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(1);
    }

    printf("Serveur UDP en attente de demandes...\n");

    // Réception du nombre n du client
    if (recvfrom(sockfd, &n, sizeof(n), 0, (struct sockaddr *)&client_addr, &client_addr_len) < 0) {
        perror("recvfrom failed");
        close(sockfd);
        exit(1);
    }
    printf("Nombre reçu du client : %d\n", n);

    // Génération de n nombres aléatoires
    int random_numbers[n];
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        random_numbers[i] = rand() % NMAX + 1;
    }

    // Envoi des n nombres aléatoires au client
    if (sendto(sockfd, random_numbers, sizeof(random_numbers), 0, (struct sockaddr *)&client_addr, client_addr_len) < 0) {
        perror("sendto failed");
        close(sockfd);
        exit(1);
    }

    printf("%d nombres aléatoires envoyés au client.\n", n);

    close(sockfd);
    return 0;
}
