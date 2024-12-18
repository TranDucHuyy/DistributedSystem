// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char filename[256];
    char buffer[BUFFER_SIZE];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address (connect to proxy)
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Proxy's IP address

    // Connect to the proxy server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Prompt user for the filename to request
    printf("Enter the filename to request from the server: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0; // Remove newline character

    // Send filename request to the proxy
    send(sock, filename, strlen(filename), 0);
    printf("Requested file: %s\n", filename);

    // Receive file content from the proxy (forwarded from server)
    FILE *file = fopen(filename, "wb");  // Create the file to save on client machine
    if (!file) {
        perror("Failed to open file to write");
        close(sock);
        exit(EXIT_FAILURE);
    }

    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, sizeof(char), bytes_received, file);
    }

    if (bytes_received < 0) {
        perror("Error receiving data");
    } else {
        printf("File received successfully and saved as: %s\n", filename);
    }

    // Close file and socket
    fclose(file);
    close(sock);
    return 0;
}
