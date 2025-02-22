#!/bin/bash

# Compilation du client
gcc clientUDP.c -o clientUDP
if [ $? -eq 0 ]; then
    echo "Compilation du client réussie."
else
    echo "Erreur lors de la compilation du client."
    exit 1
fi

# Compilation du serveur
gcc serveurUDP.c -o serveurUDP
if [ $? -eq 0 ]; then
    echo "Compilation du serveur réussie."
else
    echo "Erreur lors de la compilation du serveur."
    exit 1
fi

echo "Compilation terminée."
