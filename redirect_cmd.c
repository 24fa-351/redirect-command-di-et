#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <inp> <cmd> <out>\n", argv[0]);
        exit(1);
    }

    char *inp = argv[1];
    char *cmd = argv[2];
    char *out = argv[3];

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {

        if (strcmp(inp, "-") != 0) {
            int fd_in = open(inp, O_RDONLY);
            if (fd_in == -1) {
                perror("open input file");
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (strcmp(out, "-") != 0) {
            int fd_out = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out == -1) {
                perror("open output file");
                exit(1);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        char *args[10]; 
        int i = 0;
        args[i] = strtok(cmd, " ");
        while (args[i] != NULL) {
            i++;
            args[i] = strtok(NULL, " ");
        }

        execvp(args[0], args);
        perror("exec failed"); 
        exit(1);
    } else {
        wait(NULL);
    }

    return 0;
}
