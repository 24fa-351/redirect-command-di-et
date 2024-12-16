#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define CAN_EXECUTE(PATH) (PATH != NULL && access(PATH, X_OK) == 0)

void add_character_to_string(char *str, char chr)
{
    int len = strlen(str);
    str[len] = chr;
    str[len + 1] = '\0';
}

void split(char *cmd, char *words[], char delimiter)
{
    int word_count = 0;
    char *next_char = cmd;
    char current_word[1000];
    strcpy(current_word, "");

    while (*next_char != '\0')
    {
        if (*next_char == delimiter) // if (*next_char == ':')
        {
            words[word_count++] = strdup(current_word);
            strcpy(current_word, cmd);
        }
        else
        {
            add_character_to_string(current_word, *next_char);
        }
        ++next_char;
    }
    words[word_count++] = strdup(current_word);
    words[word_count] = NULL;
}

bool find_absolute_path(char *cmd, char *absolute_path)
{
    char *directories[1000];
    split(getenv("PATH"), directories, ':');
    for (int ix = 0; directories[ix] != NULL; ix++)
    {
        char path[1000];
        strcpy(path, directories[ix]);
        add_character_to_string(path, '/');
        strcat(path, cmd);
        if (access(path, X_OK) == 0)
        {
            strcpy(absolute_path, path);
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage: %s <inputfile> <outputfile> <command>\n", argv[0]);
        return 1;
    }

    int input_fd;
    bool redirect_input;
    if (strcmp(argv[1], "-") == 0)
    {
        redirect_input = false;
    }
    else
    {
        redirect_input = true;

        input_fd = open(argv[1], O_RDONLY | S_IRUSR | S_IWUSR); //
        if (input_fd == -1)
        {
            fprintf(stderr, "Failed to open %s for input\n", argv[1]);
            return 1;
        }
    }

    int output_fd;
    bool redirect_output;
    if (strcmp(argv[2], "-") == 0)
    {
        redirect_output = false;
    }
    else
    {
        redirect_output = true;
        output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC | S_IRUSR | S_IWUSR);
        if (output_fd == -1)
        {
            fprintf(stderr, "Failed to open %s for writing\n", argv[2]);
            return 1;
        }
    }

    char **newargv = (char **)malloc(sizeof(char *) * (1 + argc - 2)); // 3 is the number of arguments before the command

    for (int ix = 3; ix < argc; ix++)
    {
        printf("copying '%s' to newargv[%d]\n", argv[ix], ix - 3);
        newargv[ix - 3] = (char *)argv[ix];
        printf("newargv[%d] = '%s'\n", ix - 3, newargv[ix - 3]);
    }
    newargv[argc - 3] = NULL; // ./redirect inptfile outputfile "wc" "-l"

    char *executable_path = NULL;
    if (newargv[0][0] == '/' || newargv[0][0] == '.')
    {
        executable_path = strdup(newargv[0]);
    }
    else
    {
        executable_path = (char *)malloc(1000);
        if (!find_absolute_path(newargv[0], executable_path))
        {
            fprintf(stderr, "Command not found\n");
            free(executable_path);
            return 1;
        }
    }

    int child_pid = fork();
    if (child_pid == 0)
    {
        if (redirect_input)
        {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (redirect_output)
        {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        execve(executable_path, newargv, NULL);

        //additions made 12/15/2024 ----------

        // TODO: handle execve failure
        perror("execve failed");
        free(executable_path);
        exit(1);
        // fprintf(stderr, "Failed to execute %s\n", newargv[0]);
        // return 1;
    }
    else if(child_pid > 0)
    {
        // free(executable_path);
        if(redirect_input)
        {
            close(input_fd); // close the input file descriptor in the parent
        }
        if(redirect_output)
        {
            close(output_fd); // close the output file descriptor in the parent
        }
        wait(NULL);
    }
    else
    {
        perror("fork failed");
        if(redirect_input)
        {
            close(input_fd); // close the input file descriptor in the parent
        }
        if(redirect_output)
        {
            close(output_fd); // close the output file descriptor in the parent
        }
        return 1;
    }

    // end of additions -----------------------

    wait(NULL);
    printf("%s pid is %d. forked %d. "
           " parent exiting\n",
           argv[0], getpid(), child_pid);

    // free memory
    free(newargv);
    free(executable_path);

    return 0;
}
