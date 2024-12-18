// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char filename[256];
    char buffer[BUFFER_SIZE];

    // Create server socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Accept connections on any IP address

    // Bind the server socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_sock, 5) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept connection from proxy
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
    if (client_sock < 0) {
        perror("Accept failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Receive the filename from the proxy
    int n = recv(client_sock, filename, sizeof(filename), 0);
    if (n < 0) {
        perror("Error receiving filename");
        close(client_sock);
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    filename[n] = '\0';  // Null-terminate the filename string
    printf("Requested file: %s\n", filename);

    // Open the requested file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("File not found");
        close(client_sock);
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Send the file content to the proxy (which forwards to the client)
    while ((n = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0) {
        send(client_sock, buffer, n, 0);
    }

    // Close file and socket
    fclose(file);
    close(client_sock);
    close(server_sock);
    return 0;
}
