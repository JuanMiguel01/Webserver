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
#define MAX_CLIENTS 100

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

    signal(SIGPIPE, SIG_IGN);
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
    fd_set readfds;
    int max_fd;
    int client_sockets[MAX_CLIENTS] = {0};

    while (1)
    {
        // Limpiar el conjunto de descriptores de archivo
        FD_ZERO(&readfds);

        // Agregar el descriptor de archivo del socket del servidor al conjunto
        FD_SET(server_socket, &readfds);
        max_fd = server_socket;

        // Agregar los descriptores de archivo de los sockets de los clientes al conjunto
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int client_socket = client_sockets[i];
            if (client_socket > 0)
            {
                FD_SET(client_socket, &readfds);
            }
            if (client_socket > max_fd)
            {
                max_fd = client_socket;
            }
        }

        // Esperar actividad en uno de los descriptores de archivo
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // Si hay actividad en el descriptor de archivo del socket del servidor,
        // significa que hay una nueva conexión entrante
        if (FD_ISSET(server_socket, &readfds))
        {
            int new_socket = accept(server_socket, NULL, NULL);
            if (new_socket < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Agregar el nuevo socket al arreglo de sockets de clientes
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }

        // Si hay actividad en alguno de los descriptores de archivo de los sockets
        // de los clientes, significa que hay datos entrantes desde un cliente
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int client_socket = client_sockets[i];
            if (FD_ISSET(client_socket, &readfds))
            {
                FD_CLR (client_socket, &readfds);
                client_sockets [i] = 0;

                // Procesar la petición del cliente de manera asíncrona
                pthread_t thread;
                struct thread_args *targs = malloc(sizeof(struct thread_args));
                targs->new_socket = client_socket;
                targs->root_directory = root_directory;
                pthread_create(&thread, NULL, process_request, targs);
            }
        }
    }

    return 0;
}