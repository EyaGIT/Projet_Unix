#!/bin/bash

# Fonction pour compiler les programmes
compile() {
    echo "Compilation du client..."
    gcc clientTCP.c -o clientTCP
    if [ $? -eq 0 ]; then
        echo "Compilation du client réussie."
    else
        echo "Erreur lors de la compilation du client."
        exit 1
    fi

    echo "Compilation du serveur..."
    gcc serveurTCP.c -o serveurTCP
    if [ $? -eq 0 ]; then
        echo "Compilation du serveur réussie."
    else
        echo "Erreur lors de la compilation du serveur."
        exit 1
    fi
}

# Fonction pour lancer le serveur dans un terminal séparé
run_server() {
    echo "Lancement du serveur sur le port 12345 dans un nouveau terminal..."
    gnome-terminal --title="Serveur" -- bash -c "./serveurTCP 12345; exit" &
    SERVER_PID=$!
    echo "Serveur lancé avec le PID $SERVER_PID."
}

# Fonction pour lancer 2 clients dans des terminaux séparés
run_clients() {
    echo "Lancement de 2 clients dans des terminaux séparés..."

    # Adresse IP et port du serveur
    SERVER_IP="127.0.0.1"
    SERVER_PORT="12345"

    # Lancer le premier client dans un nouveau terminal
    gnome-terminal --title="Client 1" -- bash -c "./clientTCP $SERVER_IP $SERVER_PORT; exit" &

    # Lancer le deuxième client dans un nouveau terminal
    gnome-terminal --title="Client 2" -- bash -c "./clientTCP $SERVER_IP $SERVER_PORT; exit" &

    echo "2 clients lancés."
}



# Menu principal
while true; do
    echo "Choisissez une option :"
    echo "1. Compiler les programmes"
    echo "2. Lancer le serveur"
    echo "3. Lancer 2 clients"
    echo "4. Quitter"
    read -p "Entrez votre choix (1-4) : " choice

    case $choice in
        1)
            compile
            ;;
        2)
            run_server
            ;;
        3)
            run_clients
            ;;
    
        4)
            echo "Au revoir !"
            exit 0
            ;;
        *)
            echo "Choix invalide. Veuillez réessayer."
            ;;
    esac
done
