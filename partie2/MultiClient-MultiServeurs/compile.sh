#!/bin/bash

# Compiler les serveurs
echo "Compiling servers..."

gcc proxy.c -o proxy
gcc ls_svc.c -o ls_svc
gcc date_svc.c -o date
gcc fich_svc.c -o fich
gcc cnx_svc.c -o cnx
#compiler le client
gcc -o client clientTCP.c `pkg-config --cflags --libs gtk+-3.0`

# Check if compilation was successful
if [ $? -ne 0 ]; then
  echo "Compilation failed. Exiting."
  exit 1
fi

echo "Compilation complete. Launching servers..."

# Launch the proxy server in a new terminal
gnome-terminal -- bash -c "./proxy; exec bash"

# Launch the ls_svc server in a new terminal
gnome-terminal -- bash -c "./ls_svc; exec bash"

# Launch the date_svc server in a new terminal
gnome-terminal -- bash -c "./date; exec bash"

# Launch the fich_svc server in a new terminal
gnome-terminal -- bash -c "./fich; exec bash"

# Launch the cnx_svc server in a new terminal
gnome-terminal -- bash -c "./cnx; exec bash"

# Launch the client
gnome-terminal -- bash -c "./client; exec bash"

