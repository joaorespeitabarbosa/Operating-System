#include "operations.h"

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);
    //unlink
    //mkfifo fazer verificacao(!=0)

    /* TO DO */

    return 0;
}