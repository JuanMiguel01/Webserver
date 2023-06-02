#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <pthread.h>
#include "Process_and_handler_request.h"
#include "generate_directory.h"
#define BLOCK_SIZE 1024 * 4
/*
void process_request1(int new_socket, char *buffer, char *root_directory)
{
    char line[PATH_MAX] = {0};

    // Leer y procesar la primera línea
    if (sscanf(buffer, "%[^\n]", line))
    {
        char method[16] = {0}, path[1024] = {0}, version[16] = {0};
        sscanf(line, "%s %s %s", method, path, version);

        // Leer y procesar las líneas siguientes de encabezados
        char *p = buffer;
        while (sscanf(p, "%[^\n]", line))
        {

            // Buscar la cadena ": "
            char *sep = strstr(line, ": ");
            if (sep)
            {

                // Separar el nombre del encabezado del valor
                *sep = '\0';
                char *header = line;
                char *value = sep + 2;

                // Desplegar el encabezado y el valor
                // printf("%s: %s\n", header, value);
            }

            // Mover el puntero del buffer hacia adelante
            p = strchr(p, '\n');
            if (p)
                p++;
            else
                break;
        }

        if (strcmp(method, "GET") == 0)
        {
            char header[PATH_MAX] = {0};
            char resolvedPath[PATH_MAX] = {0};

            strcpy(header, root_directory);
            strcat(header, path);
            realpath(header, resolvedPath);
            printf("resolvedPath %s\n", resolvedPath);

            char *html = generate_directory_listing(resolvedPath);

            if (html != NULL)
            {
                int header_length = snprintf(header, sizeof(header),
                                             "HTTP/1.1 200 OK\r\n"
                                             "Content-Type: text/html\r\n"
                                             "Content-Length: %i\r\n"
                                             "Connection: close\r\n"
                                             "\r\n",
                                             strlen(html));
                send(new_socket, header, strnlen(header, sizeof(header)), 0);
                send(new_socket, html, strlen(html), 0);
                printf("%s", header);

                free(html);
            }
            else
            {
                int header_length = snprintf(header, sizeof(header),
                                             "HTTP/1.1 500 Internal Server Error\r\n"
                                             "Content-Type: text/html\r\n"
                                             "Content-Length: %i\r\n"
                                             "\r\n"
                                             "<html><body><h1>500 Internal Server Error</h1></body></html>\r\n",
                                             sizeof("<html><body><h1>500 Internal Server Error</h1></body></html>\r\n"));
                send(new_socket, header, header_length, 0);
            }
        }
    }
}
*/
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

void *process_request(void *args)
{
    struct thread_args *targs = (struct thread_args *)args;
    int new_socket = targs->new_socket;
    char *root_directory = targs->root_directory;
    char line[PATH_MAX] = {0};
    char buffer[1024 * 4] = {0};
    read(new_socket, buffer, sizeof(buffer));

    // Leer y procesar la primera línea
    if (sscanf(buffer, "%[^\n]", line))
    {
        char method[16] = {0}, path[1024] = {0}, version[16] = {0};
        sscanf(line, "%s %s %s", method, path, version);

        // Leer y procesar las líneas siguientes de encabezados
        char *p = buffer;
        while (sscanf(p, "%[^\n]", line))
        {
            // Buscar la cadena ": "
            char *sep = strstr(line, ": ");
            if (sep)
            {
                // Separar el nombre del encabezado del valor
                *sep = '\0';
                char *header = line;
                char *value = sep + 2;

                // Desplegar el encabezado y el valor
                // printf("%s: %s\n", header, value);
            }

            // Mover el puntero del buffer hacia adelante
            p = strchr(p, '\n');
            if (p)
                p++;
            else
                break;
        }

        if (strcmp(method, "GET") == 0)
        {
            char header[PATH_MAX] = {0};
            char resolvedPath[PATH_MAX] = {0};

            strcpy(header, root_directory);
            strcat(header, path);
            realpath(header, resolvedPath);
            printf("resolvedPath %s\n", resolvedPath);

            struct stat file_info;
            if (stat(resolvedPath, &file_info) == 0)
            {
                if (S_ISDIR(file_info.st_mode))
                {
                    // Es un directorio
                    char *html = generate_directory_listing(resolvedPath);

                    if (html != NULL)
                    {
                        int header_length = snprintf(header, sizeof(header),
                                                     "HTTP/1.1 200 OK\r\n"
                                                     "Content-Type: text/html\r\n"
                                                     "Content-Length: %i\r\n"
                                                     "Connection: close\r\n"
                                                     "\r\n",
                                                     strlen(html));
                        send(new_socket, header, strnlen(header, sizeof(header)), 0);
                        send(new_socket, html, strlen(html), 0);
                        printf("%d", strlen(html));
                        printf("%s", header);

                        free(html);
                    }
                    else
                    {
                        int header_length = snprintf(header, sizeof(header),
                                                     "HTTP/1.1 500 Internal Server Error\r\n"
                                                     "Content-Type: text/html\r\n"
                                                     "Content-Length: %i\r\n"
                                                     "\r\n"
                                                     "<html><body><h1>500 Internal Server Error</h1></body></html>\r\n",
                                                     sizeof("<html><body><h1>500 Internal Server Error</h1></body></html>\r\n"));
                        send(new_socket, header, header_length, 0);
                    }
                }
                else if (S_ISREG(file_info.st_mode))
                {
                    // Es un archivo regular
                    FILE *file = open(resolvedPath, O_RDONLY);
                    if (file != -1)
                    {
                        // Obtener el tamaño del archivo
                        off_t file_size = file_info.st_size;

                        // Enviar los encabezados HTTP
                        int header_length = snprintf(header, sizeof(header),
                                                     "HTTP/1.1 200 OK\r\n"
                                                     "Content-Type: application/octet-stream\r\n"
                                                     "Content-Disposition: attachment; filename=\"%s\"\r\n"
                                                     "Content-Length: %ld\r\n"
                                                     "Connection: close\r\n"
                                                     "\r\n",
                                                     path, file_size);
                        send(new_socket, header, header_length, 0);

                        // Enviar el contenido del archivo en bloques
                        off_t offset = 0;
                        while (offset < file_size)
                        {
                            ssize_t bytes_sent = sendfile(new_socket, file, &offset, BLOCK_SIZE);
                            if (bytes_sent <= 0)
                                break;
                        }

                        close(file);
                    }
                }
            }
        }
    }
    close(new_socket);
    free(args);
}