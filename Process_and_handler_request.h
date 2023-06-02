#ifndef PROCESS_HANDLER_H
#define PROCESS_HANDLER_H
struct thread_args {
    int new_socket;
    char *root_directory;
};

void *process_request(void *args);


#endif