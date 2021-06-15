#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>


int split(char input_str[], char *cmds[]);
void take_token(char cmd[], char *token[], char *file[]);
void exec(char cmd[]);
void strcpy1(char *des, char*src)
{
	int i;
	for(i=0;src[i]!='\0';i++)
	{
		des[i] = src[i];
	}
	des[i] = '\0'; // convert the last char to '\0'
}

int main()
{
    int inputsize = 1024; // input max size
    char input[inputsize]; // input string
    char *cmds[10]; // save split string
    int fd[10][2];
    int i;
    int pid;
    int cmd_total; // total number of procedure
    int stdin_copy, stdout_copy;

    stdin_copy= dup(STDIN_FILENO); // copy file's description
    stdout_copy = dup(STDOUT_FILENO);

    while(1)
    {
        printf("Jason/path$ ");
        fgets(input, inputsize, stdin);
        cmd_total = split(input, cmds);

        if (cmd_total > 1)
        {
            if (pipe(fd[0]) < 0)
            {
                perror("pipe");
                exit(1);
            }
        }

        i = 0;
        while (cmds[i] != NULL)
        {
            if(i > cmd_total)
            {
                break;
            }
            pid = fork();
            if (pid < 0)
            {
                perror("fork");
                exit(1);
            }
            else if(pid == 0)  // child process
            {
                if (i > 0) // it has former description, it need read last in
                {
                    close(fd[i-1][1]);
                    dup2(fd[i-1][0], STDIN_FILENO);
                }
                if (cmds[i+1] != NULL) // it has next description, it need write in
                {
                    close(fd[i][0]);
                    dup2(fd[i][1], STDOUT_FILENO);
                }
                else // the last description
                {
                    dup2(stdout_copy, STDOUT_FILENO);
                }
                exec(cmds[i]);
            }
            else // parent process
            {
                if(cmds[i+1] != NULL && cmds[i+2] != NULL)
                {
                    if (pipe(fd[i+1]) < 0)
                    {
                        perror("pipe");
                        exit(1);
                    }
                }
                if (i > 0)
                {
                    close(fd[i-1][0]); // close parent
                    close(fd[i-1][1]);
                }
                waitpid(pid, NULL, 0); // wait for child process finish
                i++;
            }
        }
    }
    return 0;
}

int split(char input_str[], char *cmds[])
{
    int i = 0;
    char *str = NULL, *saveptr = NULL;
    char *enter = NULL;
    char cmds_copy[100]; // new char to prevent modify the origin cmds
    char first_direction[100]; // string to first char to > or <
    char cmds_rdirection[100];
    char cmds_ldirection[100];
    char others_direction[10][100]; // string after > or <
    int cmds_len = 0; // count for string

    for (i=0, str=input_str; ; i++, str=NULL)
    {
        cmds[i] = strtok_r(str, "|", &saveptr); // split |
        if (cmds[i] == NULL)
        {
            enter = strrchr(cmds[i-1], '\n');
            *enter = ' ';   // convert \n to " "
            break;
        }
    }

    int check_redirection = 0;
    for(int i=0;;i++)
    {
        if(cmds[i]==NULL)
        {
            check_redirection = i;
            break;
        }
    }

    if(check_redirection > 1)
    {
        return i;
    }

    strcpy1(cmds_copy,*cmds);

    int kr=0;
    for(int i=0;i<strlen(cmds_copy);i++)  // compute >
    {
        if(cmds_copy[i] == '>')
        {
            cmds_rdirection[kr] = i;
            kr++;
        }
        if(i == strlen(cmds_copy)-1)
        {
            cmds_rdirection[kr] = '\0';
        }
    }

    int kl=0;
    for(int i=0;i<strlen(cmds_copy);i++)  // compute <
    {
        if(cmds_copy[i] == '<')
        {
            cmds_ldirection[kl] = i;
            kl++;
        }
        if(i == strlen(cmds_copy)-1)
        {
            cmds_ldirection[kl] = '\0';
        }
    }

    if(check_redirection == 1 & strlen(cmds_rdirection) == 0 & strlen(cmds_ldirection) == 0)
    {
        cmds[0] = cmds_copy;
        return 1;
    }

    if(check_redirection == 1 & strlen(cmds_ldirection) > 0) // split < into string
    {
        for(int h=0;h<=cmds_ldirection[0];h++)  // string for first <
        {
            first_direction[h] = cmds_copy[h];
            if(h == cmds_ldirection[0])
            {
                first_direction[h+1] = '\0';
            }
        }

        if(strlen(cmds_ldirection) == 1) // only one <
        {
            int k = strlen(first_direction);
            for(int j=cmds_ldirection[0]+1;j<strlen(cmds_copy);j++)
            {
                first_direction[k] = cmds_copy[j];
                k++;
                if(j == strlen(cmds_copy)-1)
                {
                    first_direction[k] = '\0';
                }
            }
            cmds[0] = first_direction;
            return 1;
        }

        if(strlen(cmds_ldirection) >= 1)
        {
            for(int i=0;i<=cmds_ldirection[0];i++)
            {
                first_direction[i] = cmds_copy[i];
                if(i == cmds_ldirection[0])
                {
                    first_direction[i+1] = '\0';
                }
            }
        }

        int char_set = 0;
        for(int i=0;i<strlen(cmds_ldirection);i++)  // connect the string
        {
            if(i <= strlen(cmds_ldirection)-2) // the first case
            {
                strcpy1(others_direction[char_set],first_direction);
                int k = strlen(others_direction[char_set]);
                for(int j=cmds_ldirection[i]+1;j<cmds_ldirection[i+1];j++)
                {
                    others_direction[char_set][k] = cmds_copy[j];
                    k ++;
                    if(j == cmds_ldirection[i+1]-1)
                    {
                        others_direction[char_set][k] = '\0';
                    }
                }
                cmds[cmds_len] = others_direction[char_set];
                char_set ++;
                cmds_len ++;
            }

            if(i == strlen(cmds_ldirection)-1) // the last case
            {
                strcpy1(others_direction[char_set],first_direction);
                int k = strlen(others_direction[char_set]);
                for(int j=cmds_ldirection[i]+1;j<strlen(cmds_copy);j++)
                {
                    others_direction[char_set][k] = cmds_copy[j];
                    k ++;
                    if(j == strlen(cmds_copy)-1)
                    {
                        others_direction[char_set][k] = '\0';
                    }
                }
                cmds[cmds_len] = others_direction[char_set];
                char_set ++;
                cmds_len ++;
            }
        }
    }

    if(check_redirection == 1 & strlen(cmds_rdirection) > 0) // split > into string
    {
        for(int h=0;h<=cmds_rdirection[0];h++) // string for first >
        {
            first_direction[h] = cmds_copy[h];
            if(h == cmds_rdirection[0])
            {
                first_direction[h+1] = '\0';
            }
        }

        if(strlen(cmds_rdirection) == 1) // only one >
        {
            int k = strlen(first_direction);
            for(int j=cmds_rdirection[0]+1;j<strlen(cmds_copy);j++)
            {
                first_direction[k] = cmds_copy[j];
                k++;
                if(j == strlen(cmds_copy)-1)
                {
                    first_direction[k] = '\0';
                }
            }
            cmds[0] = first_direction;
            return 1;
        }

        if(strlen(cmds_rdirection) >= 1)
        {
            for(int i=0;i<=cmds_rdirection[0];i++)
            {
                first_direction[i] = cmds_copy[i];
                if(i == cmds_rdirection[0])
                {
                    first_direction[i+1] = '\0';
                }
            }
        }

        int char_set = 0;
        for(int i=0;i<strlen(cmds_rdirection);i++) // connect the string
        {
            if(i <= strlen(cmds_rdirection)-2) // the first case
            {
                strcpy1(others_direction[char_set],first_direction);
                int k = strlen(others_direction[char_set]);
                for(int j=cmds_rdirection[i]+1;j<cmds_rdirection[i+1];j++)
                {
                    others_direction[char_set][k] = cmds_copy[j];
                    k ++;
                    if(j == cmds_rdirection[i+1]-1)
                    {
                        others_direction[char_set][k] = '\0';
                    }
                }
                cmds[cmds_len] = others_direction[char_set];
                char_set ++;
                cmds_len ++;
            }

            if(i == strlen(cmds_rdirection)-1) // the last case
            {
                strcpy1(others_direction[char_set],first_direction);
                int k = strlen(others_direction[char_set]);
                for(int j=cmds_rdirection[i]+1;j<strlen(cmds_copy);j++)
                {
                    others_direction[char_set][k] = cmds_copy[j];
                    k ++;
                    if(j == strlen(cmds_copy)-1)
                    {
                        others_direction[char_set][k] = '\0';
                    }
                }
                cmds[cmds_len] = others_direction[char_set];
                char_set ++;
                cmds_len++;
            }
        }
    }
    cmds_len --;
    return cmds_len; // number of procedure need to run
}

void take_args(char cmd[], char *arg[], char *file[])
{
    int i;
    char *symbol;
    char *str = NULL, *saveptr = NULL;
    int fd;
    int std_fileno;
    int mode;

    if((symbol = strrchr(cmd, '<')) != NULL) // find <
    {
        std_fileno = STDIN_FILENO;
        mode = O_RDONLY; // mode of file
    }
    else if((symbol = strrchr(cmd, '>')) != NULL) // find >
    {
        std_fileno = STDOUT_FILENO;
        mode = O_WRONLY | O_CREAT | O_TRUNC; // mode of file
    }

    if(symbol) //write into file or read from file
    {
        *symbol = '\0';
        *file = strtok_r((symbol+1), " ", &saveptr);
        fd = open(*file, mode, 0666);
        if (fd < 0)
        {
            perror("open");
            exit(1);
        }
        dup2(fd, std_fileno);
    }

    for(i=0, str=cmd, saveptr=NULL; ; i++, str=NULL) // split part of string
    {
        arg[i] = strtok_r(str, " ", &saveptr);
        if(arg[i] == NULL)
            break;
    }
    return ;
}

void exec(char cmd[])
{
    char *args[100];
    char *str, *saveptr, *file;
    int i, mode;

    take_args(cmd, args, &file);
    execvp(args[0], args);
    perror(args[0]);
    return ;
}


