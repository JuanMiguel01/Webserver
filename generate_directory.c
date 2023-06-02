#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
struct directory_entry {
    char name[256];
    off_t size;
    time_t mtime;
};

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
char *generate_directory_listing(char *root_directory,char *sort_by)
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
                       "<tr><th><a href=\"?sort_by=name\">Name</a></th><th><a href=\"?sort_by=size\">Size</a></th><th><a href=\"?sort_by=mtime\">Date</a></th></tr>\n",
                       root_directory);

    // Listar el contenido del directorio
    struct dirent *entry;
    struct directory_entry entries[1024];
    int num_entries = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        // Generar la ruta completa del archivo o directorio
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", root_directory, entry->d_name);

        // Obtener información sobre el archivo o directorio
        struct stat file_info;
        if (stat(path, &file_info) == 0)
        {
            // Almacenar la información en el arreglo de entradas
            strcpy(entries[num_entries].name, entry->d_name);
            entries[num_entries].size = file_info.st_size;
            entries[num_entries].mtime = file_info.st_mtime;
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

    // Generar la ruta completa del archivo o directorio
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", root_directory, entries[i].name);

    // Determinar si es un directorio
    DIR *dir2;
    int isdir = (closedir(dir2 = opendir(path)), dir2 != NULL);

    // Generar la fila HTML para esta entrada
    length += snprintf(html + length, buffer_size - length,
        "<tr><td><a href=\"%s%s\">%s</a></td><td>%ld</td><td>%s</td></tr>\n",
        entries[i].name, isdir ? "/" : "", entries[i].name, file_size, mod_time_str);
}
    closedir(dir);

    return html;
}
