#!/bin/bash

# Compilation du client
gcc clientTCP.c -o clientTCP
if [ $? -eq 0 ]; then
    echo "Compilation du client réussie."
else
    echo "Erreur lors de la compilation du client."
    exit 1
fi

# Compilation du serveur
gcc serveurTCP.c -o serveurTCP
if [ $? -eq 0 ]; then
    echo "Compilation du serveur réussie."
else
    echo "Erreur lors de la compilation du serveur."
    exit 1
fi

echo "Compilation terminée."
