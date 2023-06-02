#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <pthread.h>
#include "Process_and_handler_request.h"


int server_fd; // Descriptor de archivo del socket del servidor

void sigint_handler(int signum)
{
    // Cerrar el socket del servidor
    close(server_fd);

    // Salir del programa
    exit(0);
}
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s [port] [root_directory]\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    
    if (port < 1 || port > 65535)
    {
        printf("Error: Invalid port number\n");
        return 1;
    }
    char *root_directory = argv[2];
    
    DIR *dir = opendir(root_directory);
    if (dir == NULL)
    {
        printf("Error: Unable to open root directory\n");
        return 1;
    }
    closedir(dir);

    printf("Listening on port: %d\n", port);
    printf("Serving directory: %s\n", root_directory);
    signal(SIGINT, sigint_handler);

    // Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Bind the socket to a port
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("Error: Port %d is not available\n", port);
        return 1;
    }

    // Listen for connections
    listen(server_socket, 10);

    // Accept connections
    while (1)
{
    int new_socket = accept(server_socket, NULL, NULL);
    if (new_socket < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    pthread_t thread_id;
    struct thread_args *targs = malloc(sizeof(struct thread_args));
    targs->new_socket = new_socket;
    targs->root_directory = root_directory;
    pthread_create(&thread_id, NULL, process_request, targs);
}

    return 0;
}