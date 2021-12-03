#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
    Function declarations for builtin shell commands:
*/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
    list of built in commands, followed by their corresponding functions
*/
char *builtin_str[] = 
{
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) =
{
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};

int lsh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/*
    builtin function implementations
*/
int lsh_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "lsh: epexcted arguments to \"cd\"\n");
    }
    else 
    {
        if (chdir(args[1]) != 0)
        {
            perror("lsh");
        }
    }
    return -1;
}

int lsh_help(char **args)
{
    int i;
    printf("Noe Rivera's LSH\n");
    printf("Type programs names and arguments,and hit enter.\n");
    printf("The following are built in: \n");

    for (i=0; i <lsh_num_builtins(); i++)
    {
        printf (" %s\n", builtin_str[i]);
    }
    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_exit (char **args)
{
    return 0;
}

void lsh_loop(void)
{
    char *line;
    char **args;
    int status;
    
    do
    {
        printf(" > ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } 
    while (status);
}

#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(void)
{
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        //Read a charcater
        c = getchar();

        //if we hit EOF, replace it with a null character and return.
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;

        //If we have exceeded the buffer, reallocate
        if (position >= bufsize)
        {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    if (!tokens)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;
        if (position >= bufsize)
        {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens)
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int lsh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        //Child process
        if (execvp(args[0], args) == -1)
        {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        //Error forking
        perror("lsh");
    }
    else 
    {
        //Parent process
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int main(int argc, char **argv)
{
    //load config files, if any

    //run comand loop
    lsh_loop();

    //Perform any shutdown/cleanup

    return EXIT_SUCCESS;
}