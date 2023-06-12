#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

struct directory_entry
{
    char name[256];
    off_t size;
    time_t mtime;
    char permissions[11];
};

int hex_to_int(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    else if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    else
    {
        return -1;
    }
}
char *urlencode(const char *str)
{
    size_t len = strlen(str);
    char *encoded = malloc(len * 3 + 1);
    size_t i, j;
    for (i = 0, j = 0; i < len; i++)
    {
        if (isalnum(str[i]) || str[i] == '-' || str[i] == '_' || str[i] == '.' || str[i] == '~')
        {
            encoded[j++] = str[i];
        }
        else
        {
            sprintf(encoded + j, "%%%02X", (unsigned char)str[i]);
            j += 3;
        }
    }
    encoded[j] = '\0';
    return encoded;
}
char *urldecode(const char *str)
{
    size_t len = strlen(str);
    char *decoded = malloc(len + 1);
    size_t i, j;
    for (i = 0, j = 0; i < len; i++, j++)
    {
        if (str[i] == '%' && i + 2 < len && isxdigit(str[i + 1]) && isxdigit(str[i + 2]))
        {
            decoded[j] = hex_to_int(str[i + 1]) * 16 + hex_to_int(str[i + 2]);
            i += 2;
        }
        else
        {
            decoded[j] = str[i];
        }
    }
    decoded[j] = '\0';
    return decoded;
}

int compare_by_name(const void *a, const void *b)
{
    struct directory_entry *entry_a = (struct directory_entry *)a;
    struct directory_entry *entry_b = (struct directory_entry *)b;
    return strcmp(entry_a->name, entry_b->name);
}

int compare_by_size(const void *a, const void *b)
{
    struct directory_entry *entry_a = (struct directory_entry *)a;
    struct directory_entry *entry_b = (struct directory_entry *)b;
    return entry_a->size - entry_b->size;
}

int compare_by_mtime(const void *a, const void *b)
{
    struct directory_entry *entry_a = (struct directory_entry *)a;
    struct directory_entry *entry_b = (struct directory_entry *)b;
    return entry_a->mtime - entry_b->mtime;
}

void snprintf_j(char **buffer, size_t *size, size_t *offset, const char *fmt, ...)
{
    va_list l;
    va_start(l, fmt);

    size_t needed = vsnprintf(NULL, 0, fmt, l);
    va_end(l);

    while ((*size) < ((*offset) + needed + 1))
        (*size) *= 2;

    (*buffer) = realloc((*buffer), (*size));

    va_start(l, fmt);
    size_t wrote = vsnprintf((*buffer) + (*offset), (*size) - (*offset), fmt, l);
    va_end(l);

    (*offset) += wrote;
}
void strmode(mode_t mode, char *buf) {
    const char chars[] = "rwxrwxrwx";
    for (size_t i = 0; i < 9; i++) {
        buf[i] = (mode & (1 << (8-i))) ? chars[i] : '-';
    }
    buf[9] = '\0';
}

struct stat file_info;
char *generate_directory_listing(char *root_directory, char *sort_by)
{   
    DIR *dir = opendir(root_directory);
    if (dir == NULL)
    {
        return NULL;
    }

    // Inicializar el búfer HTML
    size_t buffer_size = 2;
    size_t buffer_offset = 0;
    char *html = malloc(buffer_size);
    if (html == NULL)
    {
        perror("Error al asignar memoria");
        closedir(dir);
        return NULL;
    }
    size_t length = 0;
    
    // Generar el encabezado HTML
    snprintf_j(&html, &buffer_size, &buffer_offset,
               "<!DOCTYPE html>\n"
               "<html lang=\"en\">\n"
               "<head><title>Directory Listing</title>\n"
               "<meta charset=\"utf-8\" />\n"
               "<style>\n"
               "body { font-family: Arial, sans-serif; }\n"
               "table { border-collapse: collapse; width: 100%; }\n"
               "th, td { text-align: left; padding: 8px; }\n"
               "tr:nth-child(even) { background-color: #f2f2f2; }\n"
               "th { background-color: #4CAF50; color: white; }\n"
               "a { text-decoration: none; color: #000; }\n"
               "a:hover { color: #99cc00; }\n"
               "</style>\n"
               "</head>\n"
               "<body>\n"
               "<h1>Directory: %s</h1>\n"
               "<table>\n"
               "<tr><th></th><th><a href=\"?sort_by=name\">Name</a></th><th><a href=\"?sort_by=size\">Size</a></th><th><a href=\"?sort_by=mtime\">Date</a></th><th>Permissions</a></th></tr>\n"
               "<tr><td></td><td colspan=\"3\"><a href=\"..\">&#x2190;</a></td></tr>\n",
               root_directory);

    // Listar el contenido del directorio
    struct dirent *entry;
    struct directory_entry entries[1024];
    int num_entries = 0;
    
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Generar la ruta completa del archivo o directorio
        char path[1000024];
        snprintf(path, sizeof(path), "%s/%s", root_directory, entry->d_name);

        // Obtener información sobre el archivo o directorio
        
        // Obtener información sobre el archivo o directorio
        struct stat file_info;
        if (stat(path, &file_info) == 0)
        {
            // Almacenar la información en el arreglo de entradas
            strcpy(entries[num_entries].name, entry->d_name);
            entries[num_entries].size = file_info.st_size;
            entries[num_entries].mtime = file_info.st_mtime;

            // Obtener los permisos del archivo o directorio
            mode_t permissions = file_info.st_mode;
            char permissions_str[11];
            strmode(permissions, permissions_str);

            // Almacenar los permisos en el arreglo de entradas
            strcpy(entries[num_entries].permissions, permissions_str);

            num_entries++;
            
        }
    }
    
    // Ordenar las entradas según el criterio especificado
    if (strcmp(sort_by, "name") == 0)
        qsort(entries, num_entries, sizeof(struct directory_entry), compare_by_name);
    else if (strcmp(sort_by, "size") == 0)
        qsort(entries, num_entries, sizeof(struct directory_entry), compare_by_size);
    else if (strcmp(sort_by, "mtime") == 0)
        qsort(entries, num_entries, sizeof(struct directory_entry), compare_by_mtime);

    for (int i = 0; i < num_entries; i++)
{
    // Obtener la fecha de modificación
    time_t mod_time = entries[i].mtime;
    struct tm *tm = localtime(&mod_time);
    char mod_time_str[64];
    strftime(mod_time_str, sizeof(mod_time_str), "%Y-%m-%d %H:%M:%S", tm);

    // Obtener el tamaño del archivo
    off_t file_size = entries[i].size;

    // Obtener los permisos del archivo o directorio
    char *permissions = entries[i].permissions;

    // Generar la ruta completa del archivo o directorio
    char path[4096];
    snprintf(path, sizeof(path), "%s/%s", root_directory, entries[i].name);

    // Determinar si es un directorio
    DIR *dir2;
    int isdir = (closedir(dir2 = opendir(path)), dir2 != NULL);

    // Generar la fila HTML para esta entrada
    snprintf_j (&html, &buffer_size, &buffer_offset,
        "<tr><td>%s</td><td><a href=\"%s%s\">%s</a></td><td>%ld</td><td>%s</td><td>%s</td></tr>\n",
        isdir ? "&#x1F4C2;" : "&#x1F4C4;", urlencode(entries[i].name), isdir ? "/" : "", entries[i].name, file_size, mod_time_str, permissions);
}

    closedir(dir);

    return html;
}
