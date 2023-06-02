#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

char *generate_directory_listing(char *root_directory)
{
    DIR *dir = opendir(root_directory);
    if (dir == NULL)
    {
        perror("Error al abrir el directorio");
        return NULL;
    }

    // Inicializar el búfer HTML
    size_t buffer_size = 2048;
    char *html = malloc(buffer_size);
    if (html == NULL)
    {
        perror("Error al asignar memoria");
        closedir(dir);
        return NULL;
    }
    size_t length = 0;

    // Generar el encabezado HTML
    length += snprintf(html + length, buffer_size - length,
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head><title>Directory Listing</title></head>\n"
        "<body>\n"
        "<h1>Directory: %s</h1>\n"
        "<table>\n"
        "<tr><th>Name</th><th>Size</th><th>Date</th></tr>\n",
        root_directory);

    // Listar el contenido del directorio
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Generar la ruta completa del archivo o directorio
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", root_directory, entry->d_name);

        // Obtener información sobre el archivo o directorio
        struct stat file_info;
        if (stat(path, &file_info) == 0)
        {
            // Obtener la fecha de modificación
            time_t mod_time = file_info.st_mtime;
            struct tm *tm = localtime(&mod_time);
            char mod_time_str[64];
            strftime(mod_time_str, sizeof(mod_time_str), "%Y-%m-%d %H:%M:%S", tm);

            // Obtener el tamaño del archivo
            off_t file_size = file_info.st_size;

            // Generar la fila HTML para esta entrada
            DIR *dir2;
            int isdir = (closedir(dir2 = opendir(path)), dir2 != NULL);
            length += snprintf(html + length, buffer_size - length,
                "<tr><td><a href=\"%s%s\">%s</a></td><td>%ld</td><td>%s</td></tr>\n",
                entry->d_name, isdir ? "/" : "", entry->d_name, file_size, mod_time_str);
        }
        else
        {
            // No se pudo obtener información sobre el archivo o directorio
            length += snprintf(html + length, buffer_size - length,
                "<tr><td><a href=\"%s\">%s</a></td><td>Unknown</td><td>Unknown</td></tr>\n",
                entry->d_name, entry->d_name);
        }

        // Aumentar el tamaño del búfer si es necesario
        if (length >= buffer_size - 1024)
        {
            buffer_size *= 2;
            char *new_html = realloc(html, buffer_size);
            if (new_html == NULL)
            {
                perror("Error al reasignar memoria");
                free(html);
                closedir(dir);
                return NULL;
            }
            html = new_html;
        }
    }

    // Generar el pie de página HTML
    length += snprintf(html + length, buffer_size - length,
        "</table>\n"
        "</body>\n"
        "</html>\n");

    // Cerrar el directorio
    closedir(dir);

    return html;
}
