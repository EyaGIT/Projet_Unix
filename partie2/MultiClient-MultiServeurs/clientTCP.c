#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <gtk/gtk.h>
#include "struct.h"

#define PORT 8888

int sock;
struct sockaddr_in server_addr;
msg message;

// GTK widgets
GtkWidget *window, *username_entry, *password_entry, *status_label, *file_window, *file_text_view;

// Function declarations
void show_service_window(void);
void send_request(int choice, const char *input);
void on_service_selected(GtkWidget *widget, gpointer data);
void on_login_button_clicked(GtkWidget *widget, gpointer data);
void start_gui(int argc, char *argv[]);
void show_file_content(const char *content);

// Function to send a request
void send_request(int choice, const char *input) {
    message.op = choice;
    memset(message.buff, 0, sizeof(message.buff));

    if (choice == 3 && input) {
        snprintf(message.buff, sizeof(message.buff), "%s", input);
    }

    if (send(sock, &message, sizeof(msg), 0) < 0) {
        gtk_label_set_text(GTK_LABEL(status_label), "Erreur d'envoi.");
        return;
    }

    if (recv(sock, &message, sizeof(msg), 0) < 0) {
        gtk_label_set_text(GTK_LABEL(status_label), "Erreur de réception.");
        return;
    }

    // Only display the content in the new window if the operation is "Contenu des fichiers"
    if (message.op == 3) {
        show_file_content(message.buff);
    } else {
        // Otherwise, display the result in the main status label
        gtk_label_set_text(GTK_LABEL(status_label), message.buff);
    }
}

// Function to handle service selection
void on_service_selected(GtkWidget *widget, gpointer data) {
    int choice = GPOINTER_TO_INT(data);
    
    if (choice == 5) {
        close(sock);
        gtk_main_quit();
    } else if (choice == 3) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Nom du fichier", GTK_WINDOW(window),
                                                        GTK_DIALOG_MODAL, "_OK", GTK_RESPONSE_ACCEPT, "_Annuler", GTK_RESPONSE_REJECT, NULL);
        GtkWidget *entry = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), entry, TRUE, TRUE, 0);
        gtk_widget_show(entry);
        gtk_widget_show(dialog);

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            const gchar *filename = gtk_entry_get_text(GTK_ENTRY(entry));
            send_request(choice, filename);
        }
        gtk_widget_destroy(dialog);
    } else {
        send_request(choice, NULL);
    }
}

// Function to create a window and display file content
void show_file_content(const char *content) {
    // Create a new window for displaying file content if it doesn't exist
    if (!file_window) {
        file_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(file_window), "Contenu du fichier");
        gtk_window_set_default_size(GTK_WINDOW(file_window), 400, 300);

        // Create a text view to display the content
        file_text_view = gtk_text_view_new();
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(file_text_view));
        gtk_text_view_set_editable(GTK_TEXT_VIEW(file_text_view), FALSE); // Make it non-editable
        gtk_text_buffer_set_text(buffer, content, -1); // Set the content of the file here

        GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add(GTK_CONTAINER(scroll_window), file_text_view);
        gtk_container_add(GTK_CONTAINER(file_window), scroll_window);

        gtk_widget_show_all(file_window);
    } else {
        // If the file window already exists, just update the content
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(file_text_view));
        gtk_text_buffer_set_text(buffer, content, -1);
    }
}

// Function to handle login button click
void on_login_button_clicked(GtkWidget *widget, gpointer data) {
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(username_entry));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(password_entry));

    // Prepare message for authentication
    memset(&message, 0, sizeof(msg));

    // Capture the current time for the client connection
    clock_gettime(CLOCK_REALTIME, &message.client_connection_time);

    // Set operation type to 0 (authentication)
    message.op = 0;

    // Combine the username and password in the message buffer
    snprintf(message.buff, sizeof(message.buff), "%s:%s", username, password);

    // Send the authentication message (including connection time) to the server
    if (send(sock, &message, sizeof(msg), 0) < 0) {
        gtk_label_set_text(GTK_LABEL(status_label), "Erreur d'envoi.");
        return;
    }

    // Receive response from the server
    if (recv(sock, &message, sizeof(msg), 0) < 0) {
        gtk_label_set_text(GTK_LABEL(status_label), "Erreur de réception.");
        return;
    }

    // Check the authentication result
    if (strstr(message.buff, "Authentification réussie") != NULL) {
        gtk_label_set_text(GTK_LABEL(status_label), "Connexion réussie!");
        show_service_window();
    } else {
        gtk_label_set_text(GTK_LABEL(status_label), "Échec de l'authentification.");
    }
}

// Function to display the service selection window
void show_service_window() {
    GtkWidget *service_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(service_window), "Sélection du service");
    gtk_window_set_default_size(GTK_WINDOW(service_window), 400, 300);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Choisissez un service:"), FALSE, FALSE, 5);
    
    const char *services[] = {
        "Date et heure", "Liste des fichiers", "Contenu des fichiers", "Durée de connexion", "Quitter"
    };

    for (int i = 0; i < 5; i++) {
        GtkWidget *button = gtk_button_new_with_label(services[i]);
        g_signal_connect(button, "clicked", G_CALLBACK(on_service_selected), GINT_TO_POINTER(i + 1));
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 5);
    }

    gtk_container_add(GTK_CONTAINER(service_window), vbox);
    gtk_widget_show_all(service_window);
}

// Function to initialize the GTK GUI
void start_gui(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Client GTK");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    GtkWidget *username_label = gtk_label_new("Nom d'utilisateur:");
    username_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), username_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), username_entry, 1, 0, 2, 1);

    GtkWidget *password_label = gtk_label_new("Mot de passe:");
    password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_grid_attach(GTK_GRID(grid), password_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 2, 1);

    GtkWidget *login_button = gtk_button_new_with_label("Connexion");
    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), login_button, 1, 2, 1, 1);

    status_label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), status_label, 1, 3, 2, 1);

    gtk_widget_show_all(window);
    gtk_main();
}

int main(int argc, char *argv[]) {
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(sock);
        exit(1);
    }

    start_gui(argc, argv);
    close(sock);

    return 0;
}

