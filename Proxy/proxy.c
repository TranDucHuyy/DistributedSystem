// proxy.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PROXY_PORT 8080  // Port for proxy to listen on
#define SERVER_PORT 8081 // Port for server to listen on (changed to 8081)
#define BUFFER_SIZE 1024

int main() {
    int proxy_sock, client_sock, server_sock;
    struct sockaddr_in proxy_addr, client_addr, server_addr;
    socklen_t addr_len = sizeof(client_addr);
    char filename[256];
    char buffer[BUFFER_SIZE];
    
    // Create the proxy socket
    proxy_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set proxy address to listen on port PROXY_PORT
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(PROXY_PORT);
    proxy_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the proxy socket
    if (bind(proxy_sock, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0) {
        perror("Binding failed");
        close(proxy_sock);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(proxy_sock, 5) < 0) {
        perror("Listen failed");
        close(proxy_sock);
        exit(EXIT_FAILURE);
    }

    printf("Proxy listening on port %d...\n", PROXY_PORT);

    // Accept client connection
    client_sock = accept(proxy_sock, (struct sockaddr *)&client_addr, &addr_len);
    if (client_sock < 0) {
        perror("Accept failed");
        close(proxy_sock);
        exit(EXIT_FAILURE);
    }

    // Receive filename request from client
    int n = recv(client_sock, filename, sizeof(filename), 0);
    if (n < 0) {
        perror("Error receiving filename request");
        close(client_sock);
        close(proxy_sock);
        exit(EXIT_FAILURE);
    }
    filename[n] = '\0'; // Null-terminate the filename string
    printf("Request for file: %s\n", filename);

    // Create socket to connect to the server
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Server socket creation failed");
        close(client_sock);
        close(proxy_sock);
        exit(EXIT_FAILURE);
    }

    // Set server address to connect to the server on SERVER_PORT
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);  // Change to SERVER_PORT
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Server's IP address

    // Connect to the server
    if (connect(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Server connection failed");
        close(client_sock);
        close(proxy_sock);
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Send the filename to the server
    send(server_sock, filename, strlen(filename), 0);

    // Receive the file from the server and send it to the client
    while ((n = recv(server_sock, buffer, sizeof(buffer), 0)) > 0) {
        send(client_sock, buffer, n, 0);  // Forward the data to the client
    }

    if (n < 0) {
        perror("Error receiving data from server");
    } else {
        printf("File forwarded to client successfully.\n");
    }

    // Close all sockets
    close(client_sock);
    close(proxy_sock);
    close(server_sock);
    return 0;
}
