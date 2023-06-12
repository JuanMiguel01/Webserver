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
            char *new_path;
            new_path=urldecode(path);
            strcpy(header, root_directory);
            strcat(header, new_path);

            realpath(header, resolvedPath);
            printf("resolvedPath %s\n", resolvedPath);
            char *sort_by = NULL;
            char *query_string = strchr(resolvedPath, '?');
            
            if (query_string)
            {
                query_string++;
                char *param_start = strstr(query_string, "sort_by=");
                if (param_start)
                {
                    param_start += strlen("sort_by=");
                    char *param_end = strchr(param_start, '&');
                    if (param_end)
                        sort_by = strndup(param_start, param_end - param_start);
                    else
                        sort_by = strdup(param_start);
                }
                *(query_string - 1) = '\0';
                perror(sort_by);
            }
            else
            {   
                sort_by = "name";
            }
            
            struct stat file_info;
            
            if (stat(resolvedPath, &file_info) == 0)
            {   
                if (S_ISDIR(file_info.st_mode))
                {   
                    // Es un directorio
                    char *html = generate_directory_listing(resolvedPath, sort_by);
                    
                    if (html != NULL)
                    {
                        int header_length = snprintf(header, sizeof(header),
                                                     "HTTP/1.1 200 OK\r\n"
                                                     "Content-Type: text/html\r\n"
                                                     "Content-Length: %zu\r\n"
                                                     "Connection: close\r\n"
                                                     "\r\n",
                                                     strlen(html));
                        send(new_socket, header, strnlen(header, sizeof(header)), 0);
                        send(new_socket, html, strlen(html), 0);
                        printf("%zu", strlen(html));
                        printf("%s", header);

                        free(html);
                    }
                    else
                    {
                        int header_length = snprintf(header, sizeof(header),
                                                     "HTTP/1.1 500 Internal Server Error\r\n"
                                                     "Content-Type: text/html\r\n"
                                                     "Content-Length: %zu\r\n"
                                                     "\r\n"
                                                     "<html><body><h1>500 Internal Server Error</h1></body></html>\r\n",
                                                     sizeof("<html><body><h1>500 Internal Server Error</h1></body></html>\r\n"));
                        send(new_socket, header, header_length, 0);
                    }
                }
                else if (S_ISREG(file_info.st_mode) )
                {
                    // Es un archivo regular
                    int file = open(resolvedPath, O_RDONLY);
                    if (file != -1)
                    {
                        // Obtener el tamaño del archivo
                        off_t file_size = file_info.st_size;

                        // Enviar los encabezados HTTP
                        int header_length = snprintf(header, sizeof(header),
                                                     "HTTP/1.1 200 OK\r\n"
                                                     "Content-Type: application/octet-stream\r\n"
                                                     "Content-Length: %ld\r\n"
                                                     "Connection: close\r\n"
                                                     "\r\n",
                                                     file_size);
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
            else
            {

              int header_length = snprintf(header, sizeof(header),
                                                     "HTTP/1.1 404 Not Found\r\n"
                                                     "Content-Length: %ld\r\n"
                                                     "Connection: close\r\n"
                                                     "\r\n"
                                                     "<html><body><h1>404 Not Found</h1></body></html>\r\n",
                                                     sizeof("<html><body><h1>404 Not Found</h1></body></html>\r\n"));
                        send(new_socket, header, header_length, 0);   
            }
        }
    }
    
    close (new_socket);
    free(args);
    return NULL;
}