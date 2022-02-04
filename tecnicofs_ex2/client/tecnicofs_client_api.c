#include "tecnicofs_client_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int client_session_id, f_server, f_client, f_handle, res;
ssize_t res_ld;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path)
{

    unlink(client_pipe_path);
    if (mkfifo(client_pipe_path, 0777) != 0)
    {
        return -1;
    }

    char buffer[42];
    memset(buffer, '\0', sizeof(buffer));
    memcpy(buffer, "1", sizeof(char));
    memcpy(buffer + sizeof(char), client_pipe_path, strlen(client_pipe_path));

    f_server = open(server_pipe_path, O_WRONLY);
    if (write(f_server, &buffer, sizeof(buffer)) == -1)
    {
        return -1;
    }

    f_client = open(client_pipe_path, O_RDONLY);
    if (read(f_client, &client_session_id, sizeof(client_session_id)) == -1)
    {
        return -1;
    }

    return 0;
}

int tfs_unmount()
{

    char buffer[3];
    memset(buffer, '\0', sizeof(buffer));
    memcpy(buffer, "2", sizeof(char));
    sprintf(buffer + sizeof(char), "%d", client_session_id);
    if (write(f_server, &buffer, sizeof(buffer)) == -1)
    {
        return -1;
    }

    close(f_client);
    close(f_server);

    return 0;
}

int tfs_open(char const *name, int flags)
{

    char buffer[44];
    memset(buffer, ' ', sizeof(buffer));
    memcpy(buffer, "3", sizeof(char));
    sprintf(buffer + sizeof(char), "%d", client_session_id);
    memcpy(buffer + (2 * sizeof(char)), name, sizeof(char) * strlen(name));
    memset(buffer + ((2 + strlen(name)) * sizeof(char)), ' ', (40 - strlen(name)) * sizeof(char));
    sprintf(buffer + (42 * sizeof(char)), "%d", flags);
    buffer[sizeof(buffer) - 1] = '\0';

    if (write(f_server, &buffer, sizeof(buffer)) == -1)
    {
        return -1;
    }
    if (read(f_client, &f_handle, sizeof(f_handle)) == -1)
    {
        return -1;
    }

    return f_handle;
}

int tfs_close(int fhandle)
{

    char buffer[4];
    memset(buffer, '\0', sizeof(buffer));
    memcpy(buffer, "4", sizeof(char));
    sprintf(buffer + sizeof(char), "%d%d", client_session_id, fhandle);

    if (write(f_server, &buffer, sizeof(buffer)) == -1)
    {
        return -1;
    }
    if (read(f_client, &res, sizeof(res)) == -1)
    {
        return -1;
    }

    return res;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len)
{

    char buffer_w[5 + len];
    memset(buffer_w, '\0', sizeof(buffer_w));
    memcpy(buffer_w, "5", sizeof(char));
    sprintf(buffer_w + sizeof(char), "%d%d%ld", client_session_id, fhandle, len);
    memcpy(buffer_w + (4 * sizeof(char)), buffer, sizeof(char) * strlen(buffer));

    if (write(f_server, &buffer_w, sizeof(buffer_w)) == -1)
    {
        return -1;
    }
    if (read(f_client, &res_ld, sizeof(res_ld)) == -1)
    {
        return -1;
    }

    return res_ld;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len)
{

    char buffer_r[10];
    ssize_t len_r;
    memset(buffer_r, '\0', sizeof(buffer_r));
    memcpy(buffer_r, "6", sizeof(char));
    sprintf(buffer_r + sizeof(char), "%d%d%ld", client_session_id, fhandle, len);

    if (write(f_server, &buffer_r, sizeof(buffer_r)) == -1)
    {
        return -1;
    }
    if (read(f_client, &len_r, sizeof(len_r)) == -1)
    {
        return -1;
    }
    if (read(f_client, buffer, ((size_t)len_r) * sizeof(char)) == -1)
    {
        return -1;
    }

    return len_r;
}

int tfs_shutdown_after_all_closed()
{

    char buffer[3];
    memset(buffer, '\0', sizeof(buffer));
    memcpy(buffer, "7", sizeof(char));
    sprintf(buffer + sizeof(char), "%d", client_session_id);

    if (write(f_server, &buffer, sizeof(buffer)) == -1)
    {
        return -1;
    }
    if (read(f_client, &res, sizeof(res)) == -1)
    {
        return -1;
    }

    return res;
}
