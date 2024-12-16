##
* Usage: redir <inp> <cmd> <out>

* fork and exec "cmd", redirecting the input and output descriptors as necessary. Assume the values inp and out are filenames, read from and write to them respectively. If they are "-" leave them as stdin/stdout.

* Note: cmd is ONE argument. For example "wc -l". You may need to split it into separate words (on " ")     before execing it. You will need to properly handle (in code) the case where the command is not an absolute path: find the absolute path for the command!

* No need to worry about spaces or quoted strings in its parameters. (It is not required to handle 'echo "foo bar"' as a command. That would/should/could get passed on as "echo", ""foo", "bar"".)
##

ls -l firle file two file three
argv[0] = ls
argv[1] = -l
argv[2] = firle
argv[3] = file
argv[4] = two
argv[5] = file
argv[6] = three
argc = 7

execve() (how it TAKES args)
nargv[0] = ls
nargv[1] = -l
nargv[2] = firle
nargv[3] = file
nargv[4] = two
nargv[5] = file
nargv[6] = three
nargv[7] = NULL

(there is NOT argc to use, or pass in)

./redirect inptfile outputfile "wc -l"
argv[0] = ./redirect
argv[1] = inptfile
argv[2] = outputfile
argv[3] = wc -l
argc = 4

execve() (how it TAKES args)
nargv[0] = wc
nargv[1] = -l
nargv[2] = NULL

    char *executable_path = NULL;

    if (command->terms[0][0] == '/' || command->terms[0][0] == '.')
    {
        printf("executing command: %s\n", command->terms[0]);
        executable_path = strdup(command->terms[0]);
    }
    else
    {
        executable_path = find_absolute_path(command->terms[0]);
    }

    if (!CAN_EXECUTE(executable_path))
    {
        printf("command not found\n");
        return;
    }


##