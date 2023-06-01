#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include <dirent.h>
#include "generate_directory.h"
void generate_directory_listing(int new_socket, char *root_directory) {
    DIR *dir = opendir(root_directory);
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return;
    }

    // Generate the HTML header
    char buffer[1024];
    int length = snprintf(buffer, sizeof(buffer),
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head><title>Directory Listing</title></head>\n"
        "<body>\n"
        "<h1>Directory: </h1>\n"
        "<table>\n"
        "<tr><th>Name</th><th>Size</th><th>Date</th></tr>\n",
        root_directory);
    send(new_socket, buffer, length, 0);

    // List the contents of the directory
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Generate the HTML row for this entry
        length = snprintf(buffer, sizeof(buffer),
            "<tr><td><a href=\"%s/%s\">%s</a></td><td>0</td><td>Unknown</td></tr>\n",
            root_directory, entry->d_name, entry->d_name);
        send(new_socket, buffer, length, 0);
        printf("1\n");
    }
        printf("2\n");
    // Generate the HTML footer
    length = snprintf(buffer, sizeof(buffer),
        "</table>\n"
        "</body>\n"
        "</html>\n");
    send(new_socket, buffer, length, 0);

    // Close the directory
    closedir(dir);
    printf("3\n");
}
