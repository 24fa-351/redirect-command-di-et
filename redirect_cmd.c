#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/stat.h>

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
        if (*next_char == ':')
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
        if (access(path, X_OK))
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
        printf("Usage: %s <inputfile> <outputfile> <command>\n", argv[0]); // example ./redir - "ls -l" -
        return 1;
    }

    int input_fd;
    bool redirect_input;
    if (strcmp(argv[1], "-"))
    {
        redirect_input = false;
    }
    else
    {
        redirect_input = true;

        input_fd = open(argv[1], O_RDONLY, S_IRUSR | S_IWUSR);
        if (input_fd == -1)
        {
            fprintf(stderr, "Failed to open %s for input\n", argv[1]);
            return 1;
        }
    }

    int output_fd;
    bool redirect_output;
    if (strcmp(argv[2], "-"))
    {
        redirect_output = false;
    }
    else
    {
        redirect_output = true;
        input_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (output_fd == -1)
        {
            fprintf(stderr, "Failed to open %s for writing\n", argv[2]);
            return 1;
        }
    }

    char **newargv = (char **)malloc(sizeof(char *) * (argc - 3));

    for (int ix = 3; ix < argc; ix++)
    {
        printf("copying '%s' to newargv[%d]\n", argv[ix], ix - 3);
        newargv[ix - 3] = (char*) argv[ix];
    }
    newargv[argc - 3] = NULL;

    int child_pid = fork();
    if(child_pid == 0)
    {
        if(redirect_input)
        {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if(redirect_output)
        {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        execve(newargv[0], newargv, NULL);
        // fprintf(stderr, "Failed to execute %s\n", newargv[0]);
        // return 1;
    }

    wait(NULL);
    printf("%s pid is %d. forked %d. " " parent exiting\n", argv[0], getpid(), child_pid);


    // char absolute_path[1000];
    // char *words[1000];

    // if (words[0] == NULL)
    // {
    //     printf("No command specified\n");
    //     return 1;
    // }

    // if (!find_absolute_path(words[0], absolute_path))
    // {
    //     printf("Command not found\n");
    //     return 1;
    // }

    // for (int ix = 0; words[ix] != NULL; ix++)
    // {
    //     printf("words[%d] = '%s'\n", ix, words[ix]);
    // }
    // printf("absolute_path = '%s'\n", absolute_path);

    // execve(absolute_path, words, NULL);
    // printf("execve failed\n");
    // return 1;
}
