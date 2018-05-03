#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

//Hasan Imran 11459

struct list
{
    char name[30];
    int pid;
};

static struct list process_list[30];
static int pointer=0;

int main(void)
{
    char buff[100];

    write(STDOUT_FILENO,"Use the *help* command to see how the program works\n",
          strlen("Use the *help* command to see how the program works\n"));

    while(1)
    {
        int read_count=read(STDIN_FILENO,buff,100);
        if(read_count==-1)
        {
            perror("read: ");
            exit(EXIT_FAILURE);
        }

        buff[read_count-1]='\0';

        if(read_count==1)
        {
            write(STDOUT_FILENO,"please type *help* to use the program\n",
                  strlen("please type *help* to use the program\n"));
            continue;
        }

        char* token = strtok(buff," ,\n");

        if(0 == strcmp(token, "add"))
        {
            char answer[100];
            int sum = 0;
            token = strtok(NULL, " ,\n");
            while(token != NULL)
            {
                int char_to_int = (int)atoi(token);
                sum = sum + char_to_int;
                token = strtok(NULL," ,\n");
            }
            int sprintf_count = sprintf(answer, "result of the list= %d\n", sum);
            write(STDOUT_FILENO, answer, sprintf_count);
        }

        else if(0 == strcmp(token, "sub"))
        {
            char answer[100];
            int sum=0;
            token = strtok(NULL, " ,\n");
            while(token != NULL)
            {
                int char_to_int = (int)atoi(token);
                sum = sum - char_to_int;
                token = strtok(NULL," ,\n");
            }
            int sprintf_count = sprintf(answer, "result of the list= %d\n", sum);
            write(STDOUT_FILENO, answer, sprintf_count);
        }

        else if(0 == strcmp(token, "mul"))
        {
            char answer[100];
            float sum = 1;
            token = strtok(NULL, " ,\n");
            while(token != NULL)
            {
                float char_to_float = (int)atoi(token);
                sum = sum * char_to_float;
                token = strtok(NULL," ,\n");
            }
            int sprintf_count = sprintf(answer, "result of the list= %.3f\n", sum);
            write(STDOUT_FILENO, answer, sprintf_count);
        }

        else if(0 == strcmp(token, "div"))
        {
            char answer[100];
            float sum=0;
            token = strtok(NULL, " ,\n");
            if(token != NULL)
            {
                float char_to_float = (int)atoi(token);
                sum = char_to_float;
                token = strtok(NULL, " ,\n");
            }
            while(token != NULL)
            {
                float char_to_float = (int)atoi(token);
                sum = sum / char_to_float;
                token = strtok(NULL," ,\n");
            }
            int sprintf_count = sprintf(answer, "result of the list= %.3f\n", sum);
            write(STDOUT_FILENO, answer, sprintf_count);
        }

        else if(0 == strcmp(token, "run"))
        {
            char* args[30];
            int index=0;
            token = strtok(NULL, " ,\n");
            while(token != NULL)
            {
                args[index++] = token;
                token = strtok(NULL," ,\n");
            }
            args[index]=NULL;
            int pid=fork();
            if(pid==-1)
            {
                perror("fork: ");
                exit(EXIT_FAILURE);
            }
            else if(pid>0)
            {
                pointer++;
                strcpy(process_list[pointer].name,args[0]);
                process_list[pointer].pid = pid;
                write(STDOUT_FILENO,"Parent of run fork\n",strlen("Parent of run fork\n"));
            }
            else if(pid==0)
            {
                write(STDOUT_FILENO,"Child of run fork\n",strlen("Child of run fork\n"));
                int ret = execvp(args[0], args);
                if(ret==-1)
                {
                    perror("execvp: ");
                    pointer--;
                }
            }
        }

        else if(0 == strcmp(token, "help"))
        {
            write(STDOUT_FILENO,"this is the help function\n",strlen("this is the help function\n"));
            write(STDOUT_FILENO,
                  "to use this program please enter any of the following functions and list the arguments\n",
                  strlen("to use this program please enter any of the following functions and list the arguments\n"));
            write(STDOUT_FILENO,"add num1 num2 ...\n",strlen("add num1 num2 ...\n"));
            write(STDOUT_FILENO,"sub num1 num2 ...\n",strlen("sub num1 num2 ...\n"));
            write(STDOUT_FILENO,"mul num1 num2 ...\n",strlen("mul num1 num2 ...\n"));
            write(STDOUT_FILENO,"div num1 num2 ...\n",strlen("div num1 num2 ...\n"));
            write(STDOUT_FILENO,"run program_name file for the program to open\n",
                  strlen("run program_name file for the program to open\n"));
	    write(STDOUT_FILENO,"list (this will list the processes in run)\n",
                  strlen("list (this will list the processes in run)\n"));
            write(STDOUT_FILENO,"exit (to terminate the program\n",
                  strlen("exit (to terminate the program\n"));
        }

        else if(0 == strcmp(token, "list"))
        {
            int i=1;
            char temp_list[100];
            while(i<=pointer)
            {
                int count = sprintf(temp_list, "Process %d: name: %s pid: %d\n",
                i,process_list[i].name, process_list[i].pid);
                write(STDOUT_FILENO, temp_list, count);
                i++;
            }
        }

        else if(0 == strcmp(token, "exit"))
        {
            exit(EXIT_SUCCESS);
        }

        else
        {
            write(STDOUT_FILENO,"please type *help* to use the program\n",
                  strlen("please type *help* to use the program\n"));
            continue;
        }
    }
}


