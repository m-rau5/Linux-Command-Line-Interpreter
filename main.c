#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>


int currParamAmount = 0,id;
int redirectFlag = 0; //1 > for overwrite 2 >> for append
char* redirectFile = "";
int errorFlag = 0;

void allHelp()
{
    printf("\n------------------------------------Help------------------------------------\n");
    printf("\nThe commands that needed to be implemented are:\n\n");
    printf("- nl --- Usage: nl [OPTION]... [FILE]...\n");
    printf("Write each FILE to standard output, with line numbers added.\n\n");
    printf("With no FILE, or when FILE is -, read standard input.\n\n");
    printf("Mandatory arguments to long options are mandatory for short options too.\n");
    printf("  -d, --section-delimiter=CC      use CC for logical page delimiters\n");
    printf("  -s, --number-separator=STRING   add STRING after (possible) line number\n");
    printf("      --help     display this help and exit\n");
    printf("      --version  output version information and exit\n\n");
    printf("CC are two delimiter characters used to construct logical page delimiters;\n\n");
    printf("---------------------------------------------------------------------------\n\n");
    printf("- head --- Usage: head [OPTION]... [FILE]...\n");
    printf("           Print the first 10 lines of each FILE to standard output.\n\n");
    printf("                -c, --bytes=[-]NUM       print the first NUM bytes of each file;\n");
    printf("                -c, --bytes=[-]NUM       print the first NUM bytes of each file;\n");
    printf("                -n, --lines=[-]NUM       print the first NUM lines instead\n");
    printf("                                         of the first 10;\n");
    printf("                -q, --quiet, --silent    never print headers giving file names\n");
    printf("                -v, --verbose            always print headers giving file names\n");
    printf("NUM may have a multiplier suffix:\n");
    printf("b 512, kB 1000, K 1024, MB 1000*1000, M 1024*1024,\nGB 1000*1000*1000, G 1024*1024*1024\n");
    printf("Binary prefixes can be used, too: KiB=K, MiB=M, GiB=G.\n\n");
    printf("---------------------------------------------------------------------------\n\n");
    printf("- chroot --- Usage: chroot NEWROOT [COMMAND [ARG]...]\n");
    printf("Run COMMAND with root directory set to NEWROOT.\n\n");
}

void versionInfo(){
    printf("\n-----Simple Command Line Interpreter-----\n\n");
    printf("Author: Moiș Raul\n");
    printf("West University of Timișoara\n");
    printf("Department of Computer Science\n");
    printf("2022-2023\n");
    printf("\n-----------------------------------------\n\n");
}

void headCommand(int argc, char *argv[]);

int nlCommand(int argc, char *argv[]);

void chrootCommand(int argc, char *argv[]);

void run(char **commands)
{
    if (strcmp(commands[0], "\n") == 0)
        printf("Empty command.\n");
    else if (commands[0] == NULL)
        printf("NULL command. \n");

    int id;
    if ((id = fork()) < 0)
    {
        perror("Error creating pipe for command\n");
        exit(EXIT_FAILURE);
    }
    if (id == 0)
    {
        if (execvp(commands[0], commands) == -1)
        {
            printf("%s - command was not found\n", commands[0]);
            exit(EXIT_FAILURE);
        }
    }
    else
        wait(NULL);
}

void checkRun(char **commandList){

    //check what command to do
    if (strcmp(commandList[0],"head")==0)
        headCommand(currParamAmount,commandList);
    else if (strcmp(commandList[0],"nl")==0)
        nlCommand(currParamAmount,commandList);
    else if (strcmp(commandList[0],"chroot")==0)
        chrootCommand(currParamAmount,commandList);
    else if(strcmp(commandList[0],"cd")!=0)
        run(commandList);
}

int openRedFile()
{
    int redFd;

    //open red file with append or trunc depending on flag
    if(redirectFlag == 1)
    {
        if((redFd = open(redirectFile, O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0)
        {
            perror("Error trying to open input file\n");
            errorFlag = 1;
        }
    }
    else
    {
        if((redFd = open(redirectFile, O_WRONLY | O_CREAT | O_APPEND, 0777)) < 0)
        {
            perror("Error trying to open input file\n");
            errorFlag = 1;
        }
    }

    return redFd;
}

int main(int argc, char *argv[])
{
    char *command;
    char *commandList[1000];
    int spaceNb;
    int pipedComNb;
    int status;
    int fd[pipedComNb][2];
    pid_t pidArray[pipedComNb+1];

    using_history();

    while (1)
    {
        errorFlag = 0;
        spaceNb = 0;
        pipedComNb = 0;

        // Read command
        command = readline("rex@rex-VirtualBox >> ");

        if (!command || !*command)
            continue;
        //remove spaces before command
        if(isspace(command[0]))
        {
            while(isspace(command[spaceNb]) && command[spaceNb] != '\0')
            {
                spaceNb++;
            }
            if (strlen(command) == spaceNb) //if only  at the beginning of the command ignore them
                continue;
        }

        // Add the command to the history list.
        if (strlen(command) != 0)
            add_history(command+spaceNb);


        char *pipedCommands[1000];

        char *token = strtok(command, "|");
        pipedComNb = 0;

        while (token != NULL)
        {
            pipedCommands[pipedComNb] = token;
            token = strtok(NULL, "|");
            pipedComNb++;
        }

        //printf("pipedComNb:%d\n",pipedComNb);

        for(int i = 0; i < pipedComNb; i++)
        {
            if (pipe(fd[i]) < 0)
            {
                perror("Error creating pipe");
                exit(1);
            }
        }

        ///Go thtough each of the piped commands
        for(int comNb = 0; comNb < pipedComNb; comNb++)
        {
            //printf("pipedCom:%s\n",pipedCommands[comNb]);

            ///parse commands (kinda like a costum strtok to get "" and '' strings)
            char **tokens = malloc(1000 * sizeof(char*));
            if (!tokens)
            {
                fprintf(stderr, "Error when allocating memory.\n");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < 1000; i++)
            {
                tokens[i] = malloc(1000 * sizeof(char));
                if(!tokens[i])
                {
                    fprintf(stderr, "Error when allocating memory.\n");
                    exit(EXIT_FAILURE);
                }
            }

            currParamAmount =0;
            int i = 0, tokenChrCount = 0, k = 0, redPos = -1;
            int inQuotes = 0;

            command = pipedCommands[comNb];

///---------------------------COMMAND PARSER---------------------------///

            ///Split commands in tokens and ignore spaces unless in ""
            while (command[i] != '\0')
            {
                if (inQuotes && command[i]!='"' && command[i]!='\'')
                {
                    tokens[tokenChrCount][k] = command[i];
                    k++;
                }
                else
                {
                    //if we find space set as \0
                    if (command[i] == ' ' || command[i] == '\t'|| command[i] == '\n')
                    {
                        tokens[tokenChrCount][k] = '\0';
                        tokenChrCount++;
                        k = 0;
                    }
                    //if we have the redirection simblol and it's not in quotes
                    else if(command[i] == '>' && !inQuotes) //only if last command in pipe
                    {
                        if (command[i+1] == '>')
                        {
                            i++;
                            // third > causes a unexpected token error
                            if (command[i+1] == '>')
                            {
                                printf("redirect: syntax error near unexpected token `>'\n");
                                errorFlag = 1;
                            }
                            // >> case
                            redirectFlag = 2;
                        }
                        else
                            // > case
                            redirectFlag = 1;

                        if(redPos > -1) //then we have sth like > > which is not ok
                        {
                            printf("redirect: syntax error near unexpected token `>'\n");
                            errorFlag = 1;
                        }
                        redPos = tokenChrCount+1; //set the position where the redirect file should begin
                        tokens[tokenChrCount][k] = '\0'; //ignore >
                        tokenChrCount++;
                        k = 0;

                    }
                    else
                    {
                        // store letters
                        if(command[i]!='"' && command[i]!='\'')
                        {
                            tokens[tokenChrCount][k] = command[i];
                            k++;
                        }
                    }
                }
                //if " or ' start
                if (command[i] == '\'' || command[i] == '"')
                    inQuotes = !inQuotes;

                i++;
            }
            tokens[tokenChrCount][k]='\0';

            if(command[i] == '>') //if we had the >>> unexpeted token error
                errorFlag = 1;

            if(inQuotes)
                printf("! Unclosed quotation mark !\n");

            ///reparse tokens to get command and redirection file (if redirection was used)
            for (i = 0; i <= tokenChrCount && errorFlag != 1; i++)
            {
                //if empty token space ignore it
                if(*tokens[i]!='\0' && i!=redPos)
                {
                    commandList[currParamAmount] = tokens[i];
                    currParamAmount++;
                }
                //when we reach the pos we found > or >>
                else if(i==redPos)
                {
                    //if redirect is not the last command of pipe,\
                     ignore rest of piped commands and redirect
                    if((comNb != pipedComNb-1))
                        pipedComNb = comNb;

                    //ignore spaces (stored as \0)
                    while(i <= tokenChrCount && *tokens[i]=='\0')
                        i++;

                    if(i <= tokenChrCount)
                        redirectFile = tokens[i];
                    else
                    {
                        printf("redirect: expected file to redirect to\n");
                        errorFlag =1;
                    }
                    break;
                }
            }
            i++; //counter for tokens where redirection file begins

            int redFd;

            //Open redirection file
            if(redirectFlag > 0 && errorFlag == 0)
                redFd = openRedFile();

///---------------------------COMMAND EXECUTION---------------------------///

            if(errorFlag == 1 || currParamAmount == 0)
            {
                //if error show it and don't execute the command but reset memory allocations
            }
            else if (strcmp(commandList[0], "exit") == 0)
                exit(1);
            else if (strcmp(commandList[0], "help") == 0)
                allHelp();
            else if (strcmp(commandList[0],"version")==0)
                versionInfo();
            else
            {
                //create pipe for current command
                if ((pidArray[comNb] = fork()) < 0)
                {
                    perror("Error creating pipe for command\n");
                    exit(EXIT_FAILURE);
                }
                if (pidArray[comNb] == 0)
                {
                    //if redirection copy stdout to file
                    if(redirectFlag > 0)
                    {
                        if (dup2(redFd, STDOUT_FILENO) < 0)
                        {
                            perror("Error trying to duplicate output of the command\n");
                            exit(3);
                        }
                    }

                    ///piping dups

                    ///if this is the first command
                    if(comNb > 0){
                        if(dup2(fd[comNb-1][0],STDIN_FILENO) < 0){
                            perror("Error trying to duplicate input of the previous command for piping\n");
                            exit(3);
                        }
                    }

                    ///if there still is a command after this one (aka this is not the last command)
                    if(comNb + 1 < pipedComNb){
                        if(dup2(fd[comNb][1],STDOUT_FILENO) < 0){
                            perror("Error trying to duplicate output of the command for piping\n");
                            exit(3);
                        }
                    }

                    for (int i = 0; i < pipedComNb; i++){
                        close(fd[i][0]);
                        close(fd[i][1]);
                    }

                    checkRun(commandList); //run command

                    if(redirectFlag > 0)
                        close(redFd);

                    exit(1); //exit child
                }
                else
                {
                    //in case we have cd we need to call chdir in parent process not in child
                    if(strcmp(commandList[0],"cd")==0)
                    {
                        if(currParamAmount > 1)
                        {
                            if (access(commandList[1], F_OK) == -1)
                            {
                                // file does not exist
                                fprintf(stderr, "cd: '%s': No such file or directory\n", commandList[1]);
                            }
                            else chdir(commandList[1]);
                        }
                        else
                            chdir("/home/rex");
                    }
                }
            }

            //Reset command list/params
            for(i = 0; i<currParamAmount; i++)
                commandList[i] = '\0';

            //deallocate memory for the tokens
            for(i=0; i<1000; i++)
                if(tokens[i])
                    free(tokens[i]);
            free(tokens);
        }

         //close pipes after executing all the commands we piped
        for (int i = 0; i < pipedComNb; i++){
            close(fd[i][0]);
            close(fd[i][1]);
        }

        //wait for all children to finish
        for (int i = 0; i < pipedComNb+1; i++)
            waitpid(pidArray[i], &status, 0);

        redirectFlag = 0;
        redirectFile = "";
    }

    return 0;
}
