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
    //tfs_init();

    char buffer[100];
    char aux_code[2];
    
    int i=1;
    int session_id;
    while (i==1) {
        memset(buffer, '\0', sizeof(buffer));
        memset(aux_code, '\0', sizeof(aux_code));
        int f_server = open(pipename, O_RDONLY);
        read(f_server, &buffer, sizeof(buffer));
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
                int f_client = open(sessions[session_id], O_WRONLY);
                write(f_client, &session_id, sizeof(session_id));
                break;
            
            case TFS_OP_CODE_UNMOUNT:
                session_id = atoi(buffer+sizeof(char));
                memset(sessions[session_id], '\0', sizeof(sessions[session_id]));
                break;

            case TFS_OP_CODE_OPEN:
                //TO DO
                break;
        }
    }
    /* TO DO */

    return 0;
}