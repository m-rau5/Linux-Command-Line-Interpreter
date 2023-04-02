#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int qvFlag = 0; // if 1 -> quiet if 2 -> verbose (0 - quiet if 1 file verbose if more)
int ncFlag = 1; // if 1 -> lines if 2 -> bytes
long long num_lines = 10;
long long num_bytes = 1;

long multiplier = 1;
char* multiplierSuffixes[] = {"b","K","kB","KiB","M","MB","MiB","G","GB","GiB"};

int head_qvCheck(char *params)
{
    for(int i=1; i<strlen(params); i++)
    {
        if(params[i] == 'q')  //if len == 2 and q
            qvFlag = 1;
        else if(params[i] == 'v')
            qvFlag = 2;
        else if(params[i] == 'n' || params[i] == 'c'){
            multiplier = 1;
            return i;
        }
        else
            return -1;
    }
    return 0;
}

void head_setMulti(char* multiString)
{
    if(strcmp(multiString,"b")==0)
        multiplier = 512;
    else if(strcmp(multiString,"kB")==0)
        multiplier = 1000;
    else if(strcmp(multiString,"K")==0 || strcmp(multiString,"KiB") == 0)
        multiplier = 1024;
    else if(strcmp(multiString,"MB")==0)
        multiplier = 1000*1000;
    else if(strcmp(multiString,"M")==0 || strcmp(multiString,"MiB") == 0)
        multiplier = 1024*1024;
    else if(strcmp(multiString,"GB")==0)
        multiplier = 1000*1000*1000;
    else if(strcmp(multiString,"G")==0 || strcmp(multiString,"GiB") == 0)
        multiplier = 1024*1024*1024;
    else if(strcmp(multiString,"B") == 0)
        printf("B\n");
}

int head_NumberIsNext(char* str)
{
    char curr_char,*digits = "0123456789";
    char* temp_str = str;

    int hasMulti = 0;
    char* multiString;
    int numb;

    if(str[0] == '-')
    {
        //printf("%s\n",str);
        if(strlen(str) <2)
        {
            //then we can't have -n -number
            if(ncFlag == 1)
                fprintf(stderr, "head : invalid number of lines: ‘%s’\n", str);
            else
                fprintf(stderr, "head : invalid number of bytes: ‘%s’\n", str);
            exit(EXIT_FAILURE);
        }
        else
            str++; //skip the - and just go through the number
    }
    do
    {
        curr_char = *str;
        if(strchr(digits,curr_char) == NULL)
        {
            for(int i = 0; i<10; i++)
            {
                if(strstr(multiplierSuffixes[i],str) && strcmp(str,"i") != 0 && strcmp(str,"B") != 0)
                {
                    hasMulti = 1;
                    multiString = str;
                    break;
                }
            }

            if(hasMulti){
                head_setMulti(multiString);
                break;
            }
            else
            {
                //not number and not valid multi
                return 0;
            }
        }
        str++;
    }
    while(curr_char != '/0' && curr_char != NULL);

    //get the multiplier number from the multiplier string inputter
    if(hasMulti)
    {
        temp_str[strlen(temp_str)-strlen(str)] ='\0';
        if(strlen(temp_str)>0)
        {
            if(temp_str[0] == '-' && temp_str+1 == multiString)
                numb = -1;
            else numb = atoi(temp_str);
        }
        else
            numb = 1;
    }
    else
    {
        if(strlen(temp_str)>0)
        {
            numb = atoi(temp_str);
        }
        else
            numb = 10;
    }

    if(ncFlag == 1)
        num_lines = numb * multiplier;
    else if(ncFlag == 2)
        num_bytes = numb * multiplier;
    return 1;
}

int head_ncParamChecker(char* params)
{
    if (params[1] == 'n')
        ncFlag = 1;
    else if (params[1] == 'c')
        ncFlag = 2;

    if(strlen(params) == 2) //if only -n/-c
        return 2;

    return head_NumberIsNext(params+2);
}


void head_headerPrint(char** files,int file_index)
{
    FILE* fp;
    char line[1024];
    int i=0,n;
    long counter=0;

    do
    {
        counter = 0;
        if(file_index == 0 || (*files[i] == '-'))
        {
            //get stdin
            if((fp = stdin)==NULL)
            {
                fprintf(stderr, "head: error using standard input ‘%s’\n", files[i]);
                i++;
                continue;
            }
        }
        else if((fp = fopen(files[i], "r")) == NULL)
        {
            printf("head: cannot open '%s' for reading: No such file or directory\n",files[i]);
            i++;
            continue;
        }

        if((qvFlag == 0 && file_index > 1) || qvFlag == 2)
        {
            if(fp == stdin)
                printf("===> standard input <===\n");
            else
                printf("===> %s <===\n",files[i]);
        }

        if(ncFlag == 1 && num_lines < 0 )
        {
            while(fgets(line, sizeof(line), fp) != NULL)
                counter++;

            if((n = fclose(fp)) < 0){
                printf("Error closing the file\n");
                return;
            }

            if(file_index == 0 || (*files[i] == '-'))
            {
                if((fp = stdin)==NULL)
                {
                    fprintf(stderr, "head: error using standard input ‘%s’\n", files[i]);
                    i++;
                    continue;
                }
            }
            else if((fp = fopen(files[i], "r")) == NULL)
            {
                printf("head: cannot open '%s' for reading: No such file or directory\n",files[i]);
                i++;
                continue;
            }
        }
        else if(ncFlag == 2 && num_bytes < 0)
        {
            //in case we have negative number for -c we need to get the length of the file\
            to subtract from
            fseek(fp, 0, SEEK_END);
            counter = ftell(fp);
            fseek(fp, 0, SEEK_SET);
        }

        if(ncFlag == 1)
        {
            //printf("counter: %d\n",counter);
            for (long i = 0; i < counter + num_lines; i++)
            {
                if (fgets(line, sizeof(line), fp) == NULL)
                {
                    // End of file or error
                    break;
                }
                printf("%s", line);
            }
        }
        else
        {

            if(counter == -1){
                printf("head: no pipe to get the input from \n");
                i++;
                continue;
            }

            for (int i = 0; i < counter + num_bytes; i++)
            {
                int c = fgetc(fp);
                if (c == EOF)
                    break;
                putchar(c);
            }
        }
        i++;

        //to imitate how command line puts newline after file name
        if(qvFlag != 1 && file_index > 1 && i!=file_index){
            printf("\n");
        }
        if((n = fclose(fp)) < 0){
                printf("Error closing the file\n");
                return;
        }
    }
    while(i<file_index);
}

void headCommand(int argc, char *argv[])
{
    char **files = malloc(64 * sizeof(char *));
    int fileIndex = 0;
    for (int i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            if(strlen(argv[i]) == 1)
            {
                files[fileIndex] = argv[i];
                fileIndex++;
            }
            else if(argv[i][1] == 'q' || argv[i][1] == 'v')
            {
                int qvCheck = head_qvCheck(argv[i]);
                if(qvCheck == -1)
                {
                    fprintf(stderr, "head: invalid trailing option -- ‘%s’\n", argv[i]+2);
                    exit(EXIT_FAILURE);
                }
                else if (qvCheck >= 2) //if sth like -qvqvqvn23
                {
                    char temp[strlen(argv[i]+qvCheck)+1];
                    strcpy(temp+1,argv[i]+qvCheck);
                    temp[0] = '-';
                    //printf("%s\n",temp);
                    int ncCheck = head_ncParamChecker(temp);
                    if(ncCheck == 2)
                    {
                        if(i + 1 >= argc)
                        {
                            fprintf(stderr, "head: option requires an argument -- '%s'\n", temp);
                            exit(EXIT_FAILURE);
                        }
                        //check after n (n "number")
                        if(head_NumberIsNext(argv[i+1]) == 0)
                        {
                            if(ncFlag == 1)
                                fprintf(stderr, "head : invalid number of lines: ‘%s’\n", argv[i+1]);
                            else
                                fprintf(stderr, "head : invalid number of bytes: ‘%s’\n", argv[i+1]);
                            exit(EXIT_FAILURE);
                        }
                        else
                            i++;
                    }
                    else if(ncCheck == 0)
                    {
                        if(ncFlag == 1)
                            fprintf(stderr, "head : invalid number of lines: ‘%s’\n", argv[i]+2);
                        else
                            fprintf(stderr, "head : invalid number of bytes: ‘%s’\n", argv[i]+2);
                        exit(EXIT_FAILURE);
                    }
                }
            }
            else if(argv[i][1] == 'n' || argv[i][1] == 'c') //if -n/-c
            {
                multiplier = 1;
                int ncCheck = head_ncParamChecker(argv[i]);
                if(ncCheck == 2)
                {
                    if(i + 1 >= argc) //if command ends at -n
                    {
                        fprintf(stderr, "head: option requires an argument -- '%s'\n", argv[i]+1);
                        exit(EXIT_FAILURE);
                    }
                    //check after n (n "number")
                    if(head_NumberIsNext(argv[i+1]) == 0)
                    {
                        if(ncFlag == 1)
                            fprintf(stderr, "head : invalid number of lines: ‘%s’\n", argv[i+1]);
                        else
                            fprintf(stderr, "head : invalid number of bytes: ‘%s’\n", argv[i+1]);
                        exit(EXIT_FAILURE);
                    }
                    else
                        i++;
                }
                else if(ncCheck == 0)
                {
                    if(ncFlag == 1)
                        fprintf(stderr, "head : invalid number of lines: ‘%s’\n", argv[i]+2);
                    else
                        fprintf(stderr, "head : invalid number of bytes: ‘%s’\n", argv[i]+2);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                fprintf(stderr, "head: invalid option -- ‘%s’\n", argv[i]);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            files[fileIndex] = argv[i];
            fileIndex++;
        }
    }

    //print lines
    head_headerPrint(files,fileIndex);

    qvFlag = 0;
    ncFlag = 1; // if 1 -> lines if 2 -> bytes
    num_lines = 10,num_bytes = -1;
    multiplier = 1;
    free(files);
}
