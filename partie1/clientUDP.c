#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define NMAX 100  // Valeur maximale pour n

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in server_addr;
    int n;
    char buffer[1024];

    // Création de la socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(1);
    }

    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));  // Port du serveur
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);  // Adresse IP du serveur

    // Génération d'un nombre aléatoire n entre 1 et NMAX
    srand(time(NULL));
    n = rand() % NMAX + 1;
    printf("Nombre généré : %d\n", n);

    // Envoi du nombre n au serveur
    if (sendto(sockfd, &n, sizeof(n), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("sendto failed");
        close(sockfd);
        exit(1);
    }

    // Réception des n nombres aléatoires du serveur
    int received_numbers[n];
    socklen_t server_addr_len = sizeof(server_addr);
    if (recvfrom(sockfd, received_numbers, sizeof(received_numbers), 0, (struct sockaddr *)&server_addr, &server_addr_len) < 0) {
        perror("recvfrom failed");
        close(sockfd);
        exit(1);
    }

    // Affichage des nombres reçus
    printf("Nombres reçus du serveur : ");
    for (int i = 0; i < n; i++) {
        printf("%d ", received_numbers[i]);
    }
    printf("\n");

    close(sockfd);
    return 0;
}
