#ifndef PROCESS_HANDLER_H
#define PROCESS_HANDLER_H


void process_request(int new_socket, char *buffer,char *root_directory);
void handle_request(int new_socket,char *buffer);

#endif