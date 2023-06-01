#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include "Process_and_handler_request.h"
#include "generate_directory.h"

void process_request(int new_socket, char *buffer, char *root_directory)
{
    char line[1024];
    printf("1");
    // Leer y procesar la primera línea
    if (sscanf(buffer, "%[^\n]", line))
    {
        printf("2");
        char method[16], path[1024], version[16];
        sscanf(line, "%s %s %s", method, path, version);
        printf("3");
        // Leer y procesar las líneas siguientes de encabezados
        char *p = buffer;
        while (sscanf(p, "%[^\n]", line))
        {
            printf("4");
            // Buscar la cadena ": "
            char *sep = strstr(line, ": ");
            if (sep)
            {
                printf("5");
                // Separar el nombre del encabezado del valor
                *sep = '\0';
                char *header = line;
                char *value = sep + 2;

                // Desplegar el encabezado y el valor
                printf("%s: %s\n", header, value);
            }

            // Mover el puntero del buffer hacia adelante
            p = strchr(p, '\n');
            if (p)
                p++;
            else
                break;
        }
    }
    printf("6\n");

    // Check if the request is for a directory
    if (strstr(buffer, "GET / HTTP/1.1") != NULL)
    {
        printf("7\n");
        char header[1024];
        int header_length = snprintf(header, sizeof(header),
                                     "HTTP/1.1 200 OK\r\n"
                                     "Content-Type: text/html\r\n"
                                     "Connection: close\r\n"
                                     "\r\n");
        send(new_socket, header, header_length, 0);
        printf("%s", header);
        printf("entro al listing\n");
        generate_directory_listing(new_socket, root_directory);
        perror("termino");
    }
}

/*
void handle_request(int new_socket,char *buffer)
{
    // Check if the request is for a directory
    if (strstr(buffer, "GET /directory/ HTTP/1.1") != NULL)
    {
        // Open the directory
        DIR *dir = opendir("/directory/");

        // List the contents of the directory
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            // Send the name of the file to the client
            send(new_socket, entry->d_name, strlen(entry->d_name), 0);
        }

        // Close the directory
        closedir(dir);
    }
    else
    {
        // Send a 404 Not Found error to the client
        int error_message_length = strlen("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
        send(new_socket, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n", error_message_length, 0);
    }
}
*/