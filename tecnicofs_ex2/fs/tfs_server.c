#include "operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

char sessions[NUMBER_OF_SESSIONS][40];

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);

    unlink(pipename);
    if (mkfifo(pipename, 0777) != 0) {return -1;}
    tfs_init();

    char buffer[100];
    char aux_code[2];
    char *content;
    open_struct openbufferstruct;
    
    int i=1;
    int f_client, session_id, f_handle, result;
    size_t len;
    ssize_t result_ld;
    size_t result_lui;
    while (i==1) {
        memset(buffer, '\0', sizeof(buffer));
        memset(aux_code, '\0', sizeof(aux_code));
        int f_server = open(pipename, O_RDONLY);
        if(read(f_server, &buffer, sizeof(buffer))==-1) {return -1;}
        memcpy(aux_code, buffer, sizeof(char));
        switch(atoi(aux_code)) {
            case TFS_OP_CODE_MOUNT:
                for(int j=0; j<NUMBER_OF_SESSIONS; j++) {
                    if (strlen(sessions[j]) == 0) {
                        session_id = j;
                        j = NUMBER_OF_SESSIONS;
                    }
                }
                memcpy(sessions[session_id], buffer+sizeof(char), sizeof(sessions[session_id]));
                f_client = open(sessions[session_id], O_WRONLY);
                if(write(f_client, &session_id, sizeof(session_id))==-1) {return -1;}
                break;
            
            case TFS_OP_CODE_UNMOUNT:
                session_id = atoi(buffer+sizeof(char));
                memset(sessions[session_id], '\0', sizeof(sessions[session_id]));
                break;

            case TFS_OP_CODE_OPEN:
                memset(&openbufferstruct.name, '\0', sizeof(openbufferstruct.name));
                memcpy(&openbufferstruct.name, buffer+(2*sizeof(char)), sizeof(openbufferstruct.name));
                for (int j=0; j<sizeof(openbufferstruct.name); j++) {
                    if (openbufferstruct.name[i] == ' ') {
                        openbufferstruct.name[i] = '\0';
                        j = sizeof(openbufferstruct.name);
                    }
                }
                openbufferstruct.flag = atoi(buffer+(42*sizeof(char)));
                f_handle = tfs_open(openbufferstruct.name, openbufferstruct.flag);
                session_id = atoi(buffer+sizeof(char));
                f_client = open(sessions[session_id], O_WRONLY);
                if(write(f_client, &f_handle, sizeof(f_handle))==-1) {return -1;}
                break;

            case TFS_OP_CODE_CLOSE:
                f_handle = atoi(buffer+(2*sizeof(char)));
                result = tfs_close(f_handle);
                memcpy(aux_code, buffer+sizeof(char), sizeof(char));
                session_id = atoi(aux_code);
                f_client = open(sessions[session_id], O_WRONLY);
                if(write(f_client, &result, sizeof(result))==-1) {return -1;}
                break;
            
            case TFS_OP_CODE_WRITE:
                memcpy(aux_code, buffer+(2*sizeof(char)), sizeof(char));
                f_handle = atoi(aux_code);
                len = (size_t) atoi(buffer+(3*sizeof(char)));
                content = (char*) malloc(len*sizeof(char));
                memcpy(content, buffer+(4*sizeof(char)), len*sizeof(char));
                result_ld = tfs_write(f_handle, content, len);
                memcpy(aux_code, buffer+sizeof(char), sizeof(char));
                session_id = atoi(aux_code);
                f_client = open(sessions[session_id], O_WRONLY);
                if(write(f_client, &result_ld, sizeof(result_ld))==-1) {return -1;}
                free(content);
                break;
            
            case TFS_OP_CODE_READ:
                memcpy(aux_code, buffer+(2*sizeof(char)), sizeof(char));
                f_handle = atoi(aux_code);
                len = (size_t) atoi(buffer+(3*sizeof(char)));
                content = (char*) malloc(len*sizeof(char));
                result_lui = (size_t)tfs_read(f_handle, content, len);
                memcpy(aux_code, buffer+sizeof(char), sizeof(char));
                session_id = atoi(aux_code);
                f_client = open(sessions[session_id], O_WRONLY);
                if(write(f_client, &result_lui, sizeof(result_lui))==-1) {return -1;}
                if(write(f_client, content, result_lui*sizeof(char))==-1) {return -1;}
                free(content);
                break;

            case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED:
                result = tfs_destroy_after_all_closed();
                session_id = atoi(buffer+sizeof(char));
                f_client = open(sessions[session_id], O_WRONLY);
                if(write(f_client, &result, sizeof(result))==-1) {return -1;}
                i--;
                break;
            
            default:
                break;
        }
    }

    return 0;
}