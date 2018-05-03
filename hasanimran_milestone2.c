#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

struct list
{
   char name[30];
   int pid;
   int status;
};

static struct list process_list[30];
static int pointer=0;

static void sig_handler(int signo)
{
   int status_wait;
   int child = waitpid(-1,&status_wait,WNOHANG);
   if(signo==SIGCHLD)
   {
      if(5==WEXITSTATUS(status_wait))
      {
         pointer--;
         return;
      }
      for(int i=0; i<30; i++)
      {
         if(process_list[i].pid==child)
         {
            process_list[i].status = 0;
         }
      }
   }
}

int main(void)
{
   int pipefd[2];
   int pipefd2[2];
   pipe(pipefd);
   if (pipe(pipefd) == -1)
   {
      perror("pipe");
      exit(EXIT_FAILURE);
   }
   pipe(pipefd2);
   if (pipe(pipefd2) == -1)
   {
      perror("pipe");
      exit(EXIT_FAILURE);
   }

   int c=fork();
   if (c == -1)
   {
      perror("fork");
      exit(EXIT_FAILURE);
   }
   else if(c>0)
   {
      //client
      char to_server[999];
      char from_server[999];
      close(pipefd[0]);//closing read
      close(pipefd2[1]);//closing write
      while(1)
      {
         int read_count1=read(STDIN_FILENO,to_server,999);
         if(read_count1==-1)
         {
            perror("read: ");
            exit(EXIT_FAILURE);
         }

         to_server[read_count1-1]='\0';
         write(pipefd[1],to_server, read_count1);
         int read_count2=read(pipefd2[0],from_server,999);
         write(STDOUT_FILENO,from_server,read_count2);
      }
   }
   else
   {
      //server

      __sighandler_t prev_SIG = signal(SIGCHLD,sig_handler);
      if(*prev_SIG==SIG_ERR)
      {
         fprintf(stderr,"Cannot handle SIGCHLD!!!!!\n");
         exit(EXIT_FAILURE);
      }

      close(pipefd[1]);//closing write
      close(pipefd2[0]);//closing read
      while(1)
      {
         char from_client[999];
         char to_client[999];
         int read_count1=read(pipefd[0], from_client,999);
         if(read_count1==-1)
         {
            perror("read: ");
            exit(EXIT_FAILURE);
         }

         from_client[read_count1-1]='\0';

         if(read_count1==1)
         {
            write(pipefd2[1],"please type *help* to use the program\n",
                  strlen("please type *help* to use the program\n"));
            continue;

         }

         char* token = strtok(from_client," ,\n");

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
            write(pipefd2[1], answer, sprintf_count);
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
            write(pipefd2[1], answer, sprintf_count);
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
            write(pipefd2[1], answer, sprintf_count);
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
            write(pipefd2[1], answer, sprintf_count);
         }

         else if(0 == strcmp(token, "run"))
         {
            char* args[30];
            int c=0;
            token = strtok(NULL, " ,\n");
            while(token != NULL)
            {
               args[c++] = token;
               token = strtok(NULL," ,\n");
            }
            args[c]=NULL;
            int pid=fork();
            if(pid==-1)
            {
               perror("fork: ");
               exit(EXIT_FAILURE);
            }
            else if(pid>0)
            {
               write(STDOUT_FILENO,"Parent of run fork\n",strlen("Parent of run fork\n"));
               pointer++;
               strcpy(process_list[pointer].name,args[0]);
               process_list[pointer].pid = pid;
               process_list[pointer].status=1;
            }
            else if(pid==0)
            {
               write(STDOUT_FILENO,"Child of run fork\n",strlen("Child of run fork\n"));
               int ret = execvp(args[0], args);
               if(ret==-1)
               {
                  perror("execvp: ");
                  exit(5);
               }
            }
            write(pipefd2[1], "run successful\n", strlen("run successful\n"));
         }

         else if(0 == strcmp(token, "help"))
         {
            write(pipefd2[1],"this is the help function\nto use this program please enter any of the following functions and list the arguments\nadd num1 num2 ...\nsub num1 num2 ...\nmul num1 num2 ...\ndiv num1 num2 ...\nrun program_name file for the program to open\nlist (this will list the processes in run)\nexit (to terminate the program)\n",strlen("this is the help function\nto use this program please enter any of the following functions and list the arguments\nadd num1 num2 ...\nsub num1 num2 ...\nmul num1 num2 ...\ndiv num1 num2 ...\nrun program_name file for the program to open\nlist (this will list the processes in run)\nexit (to terminate the program)\n"));
         }

         else if(0 == strcmp(token, "list"))
         {
            token = strtok(NULL, " ,\n");
            if(token == NULL)
            {
               int i=1;
               char temp_list[100];
               while(i<=pointer)
               {
                  if(process_list[i].status == 1)
                  {
                     int count = sprintf(temp_list, "Process %d: name: %s pid: %d status: %d\n",
                                         i,process_list[i].name, process_list[i].pid, process_list[i].status);
                     write(STDOUT_FILENO, temp_list, count);
                  }
                  i++;
               }
            }
            else if(0 == strcmp(token, "all"))
            {
               int i=1;
               char temp_list[100];
               while(i<=pointer)
               {
                  int count = sprintf(temp_list, "Process %d: name: %s pid: %d status: %d\n",
                                      i,process_list[i].name, process_list[i].pid, process_list[i].status);
                  write(STDOUT_FILENO, temp_list, count);
                  i++;
               }
            }
            write(pipefd2[1], "list displayed\n", strlen("list displayed\n"));
         }

         else if(0 == strcmp(token, "exit"))
         {
            exit(EXIT_SUCCESS);
         }

         else if(0 == strcmp(token, "kill"))
         {
            token = strtok(NULL, " ,\n");
            int to_kill = (int)atoi(token);
            int is_kill = 0;
            for(int i=0; i<30; i++)
            {
               if(process_list[i].pid==to_kill)
               {
                  is_kill=1;
                  break;
               }
            }
            if(is_kill==1)
            {
               int k_id = kill(to_kill, SIGTERM);
               if(k_id==-1)
               {
                  perror("kill: ");
               }
               if(k_id==0)
               {
                  write(pipefd2[1],"kill successful\n",strlen("kill successful\n"));
               }
            }
            else
            {
               write(pipefd2[1],"kill unsuccessful\n",strlen("kill unsuccessful\n"));
            }
         }

         else if(token == NULL)
         {
            write(pipefd2[1],"please type *help* to use the program\n",
                  strlen("please type *help* to use the program\n"));
            continue;
         }

         else
         {
            write(pipefd2[1],"please type *help* to use the program\n",
                  strlen("please type *help* to use the program\n"));
            continue;
         }

      }
   }
}
