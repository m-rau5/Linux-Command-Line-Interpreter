#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int chrootCommand(int argc, char *argv[]) {
    if(argc > 1){
        //check if file exists and if not error out
        if (access(argv[1], F_OK) == -1) {
            // file does not exist
            fprintf(stderr, "chroot: cannot change root directory to '%s': No such file or directory\n", argv[1]);
            exit(1);
        }


        if ( chroot(argv[1]) != 0 ){
             perror("chroot");
             printf("Note: terminal needs to be run with sudo for chroot to work.\n");
             exit(1);
        }
    }
    else{
        printf("chroot: missing operand\n");
        exit(1);
    }

    // change the current working directory to '/'
     if (chdir("/") != 0) {
        perror("chdir");
        exit(1);
    }

    // execute the command
    ///THE CHDIR SETS THE DIRECTORY TO THE ROOT OF LINUX NOT OF CHROOT?
    if (argc > 2) { //check if command/file/script we try to run/access exitsts in the chrooted directory
        execlp(argv[2], argv[2], NULL);
    } else {
        char *bash_path = "/bin/bash";
        if (access(bash_path, F_OK) == -1) {
            // file does not exist in chrooted directory
            fprintf(stderr, "Error: %s does not exist in the chrooted directory\n", bash_path);
            exit(1);
        }
        execlp(bash_path, "bash", NULL);
    }

}
