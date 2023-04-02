#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

char* lineDelim = "\\:";
char* numSep = "  ";

int nlPrint(char** files,int fileIndex)
{
    FILE* fd;
    int i = 0,lineCount = 1,n;
    char line[1024];
    int delimCount = 0,delimBreak;

    do
    {
        if(fileIndex == 0 || (*files[i] == '-'))
        {
            if((fd = stdin) == NULL)
            {
                fprintf(stderr, "nl: error using standard input ‘%s’\n", files[i]);
                i++;
                continue;
            }
        }
        else if((fd = fopen(files[i], "r")) == NULL)
        {
            printf("nl: cannot open '%s' for reading: No such file or directory\n",files[i]);
            i++;
            continue;
        }

        while (fgets(line, 1024, fd))
        {
            // Check if the line is empty
            if(strstr(line,lineDelim) == line)
            {

                //check if line is either split in header,footer or body
                delimBreak=0;

                //check for line == "Delim" => header
                if(strlen(line)-1 == strlen(lineDelim))
                {
                    delimCount = 1;
                }
                //check for line == "DelimDelim" => body
                else if(strlen(line+strlen(lineDelim))-1 == strlen(lineDelim))
                {

                    if(strstr((line+strlen(lineDelim)),lineDelim) == line + strlen(lineDelim))
                    {
                        delimCount = 2;
                    }
                    else delimBreak = 1;
                }
                //check for line == "DelimDelimDelim" => footer
                else if(strstr(line+strlen(lineDelim)*2,lineDelim) == line + 2*strlen(lineDelim))
                {
                    if(strlen(line+strlen(lineDelim)*2)-1 == strlen(lineDelim))
                    {
                        delimCount = 3;
                    }
                    else delimBreak = 1;
                }
                else delimBreak = 1;

                if(delimBreak == 1)
                {
                    if(delimCount == 0) //default case
                        printf("%d%s%s", lineCount,numSep,line);
                    else if(delimCount == 1 || delimCount == 3) //header/footer stop line numbering
                        printf("  %s", line);
                    else if(delimCount == 2) //body resets line counter
                    {
                        lineCount = 1;
                        printf("%d%s%s", lineCount,numSep, line);
                        delimCount = 0;
                    }
                }
                else printf("\n");
                lineCount++;
            }
            else if (line[0] != '\n')
            {
                // Print the line counter and the line of input
                if(delimCount == 0)
                    printf("%d%s%s", lineCount,numSep,line);
                else if(delimCount == 1 || delimCount == 3)
                    printf("  %s", line);
                else if(delimCount == 2)
                {
                    lineCount = 1;
                    printf("%d%s%s", lineCount,numSep, line);
                    delimCount = 0;
                }
                lineCount++;
            }
            else printf("\n");
        }

        if((n = fclose(fd)) < 0){
            perror("Error closing the file");
            exit(EXIT_FAILURE);
        }

        //file is open
        i++;

    }
    while(i<fileIndex);

    i = 0;
    return 0;
}

//in terminal for -d if the arg is a character it wont see it as delimieter,
//even with : added in file, unless we use two chars in the -d argument

int nlCommand(int argc, char *argv[])
{
    char** files = malloc(64*sizeof(char));
    int fileIndex = 0,i;

    for(i=1; i<argc; i++)
    {
        //printf("%s\n",argv[i]);
        if(argv[i][0] == '-') //if option
        {
            if(strlen(argv[i]) == 1)
            {
                files[fileIndex] = argv[i];
                fileIndex++;
            }
            else if(argv[i][1] == 'd')
            {
                //printf("-d option\n");
                if(strlen(argv[i]) == 2)
                {
                   if(i+1 >= argc){
                        fprintf(stderr, "nl: option requires an argument -- ‘%s’\n", argv[i]);
                        exit(EXIT_FAILURE);
                    }
                    else{
                        lineDelim = argv[i+1];
                        i++;
                    }
                }
                else
                    lineDelim = argv[i]+2;
            }
            else if(argv[i][1] == 's')
            {
                if(strlen(argv[i]) == 2)
                {
                    if(i+1 >= argc){
                        fprintf(stderr, "nl: option requires an argument -- ‘%s’\n", argv[i]);
                        exit(EXIT_FAILURE);
                    }
                    else{
                        numSep = argv[i+1];
                        i++;
                    }
                }
                else
                    numSep = argv[i]+2;
            }
            else
            {
                fprintf(stderr, "nl: invalid option -- ‘%s’\n", argv[i]);
                exit(EXIT_FAILURE);
            }
        }
        else{
            files[fileIndex] = argv[i];
            fileIndex++;
        }
    }

    nlPrint(files,fileIndex);

    lineDelim = "\\:";
    numSep = "  ";
    free(files);

    return 0;
}
