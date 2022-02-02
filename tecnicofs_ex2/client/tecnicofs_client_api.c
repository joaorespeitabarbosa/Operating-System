#include "tecnicofs_client_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int client_session_id, f_server, f_client;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    unlink(client_pipe_path);
    if (mkfifo(client_pipe_path, 0777) != 0) {return -1;}

    char buffer[41];
    memset(buffer, '\0', sizeof(buffer));
    memcpy(buffer, "1", sizeof(char));
    memcpy(buffer+sizeof(char), client_pipe_path, strlen(client_pipe_path));

    f_server = open(server_pipe_path, O_WRONLY);
    write(f_server, &buffer, sizeof(buffer));

    f_client = open(client_pipe_path, O_RDONLY);
    read(f_client, &client_session_id, sizeof(client_session_id));

    return 0;
}

int tfs_unmount() {
    
    char buffer[3];
    memset(buffer, '\0', sizeof(buffer));
    memcpy(buffer, "2", sizeof(char));
    sprintf(buffer+sizeof(char), "%d", client_session_id);
    write(f_server, &buffer, sizeof(buffer));

    close(f_client);
    close(f_server);

    return 0;
}

int tfs_open(char const *name, int flags) {
    /* TODO: Implement this */
    return -1;
}

int tfs_close(int fhandle) {
    /* TODO: Implement this */
    return -1;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    /* TODO: Implement this */
    return -1;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    /* TODO: Implement this */
    return -1;
}

int tfs_shutdown_after_all_closed() {
    /* TODO: Implement this */
    return -1;
}
