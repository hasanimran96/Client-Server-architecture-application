#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>

struct list
{
   char name[30];
   int pid;
   int status;
   time_t start_time;
   time_t end_time;
   time_t elapsed_time;
};

static struct list process_list[30];
static int pointer=0;

static int no_of_clients=0;

static void sig_handler(int signo)
{
   int status_wait;
   int child_pid = waitpid(-1,&status_wait,WNOHANG);
   if(signo==SIGCHLD)
   {
      for(int i=0; i<30; i++)
      {
         if(process_list[i].pid==child_pid)
         {
            process_list[i].status = 0;
            process_list[i].end_time = time(NULL);
         }
      }
   }
}

static void math(char* token, int fd)
{
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
      int sprintf_count = sprintf(answer, "result of add is= %d\n", sum);
      write(fd, answer, sprintf_count);
   }
   else if(0 == strcmp(token, "sub"))
   {
      char answer[100];
      int sum=0;
      token = strtok(NULL, " ,\n");
      if(token != NULL)
      {
         int char_to_float = (int)atoi(token);
         sum = char_to_float;
         token = strtok(NULL, " ,\n");
      }
      while(token != NULL)
      {
         int char_to_int = (int)atoi(token);
         sum = sum - char_to_int;
         token = strtok(NULL," ,\n");
      }
      int sprintf_count = sprintf(answer, "result of the sub is= %d\n", sum);
      write(fd, answer, sprintf_count);
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
      int sprintf_count = sprintf(answer, "result of multiply is= %.3f\n", sum);
      write(fd, answer, sprintf_count);
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
      int sprintf_count = sprintf(answer, "result of divide is= %.3f\n", sum);
      write(fd, answer, sprintf_count);
   }
}

static void run(char* token, int fd)
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

   /*pipe will return zero on read
   and pipe will return -1 with epipe on write*/
   int pipefd3[2];
   int pipe_err3 = pipe(pipefd3);
   if(pipe_err3 == -1)
   {
      perror("pipe run");
      exit(EXIT_SUCCESS);
   }

   int fcntl_err = fcntl(pipefd3[1], F_SETFD, FD_CLOEXEC);

   __sighandler_t prev_SIGCHLD = signal(SIGCHLD,sig_handler);
   if(*prev_SIGCHLD==SIG_ERR)
   {
      fprintf(stderr,"Cannot handle SIGCHLD!!!!!\n");
      exit(EXIT_FAILURE);
   }

   int pid=fork();
   if(pid==-1)
   {
      perror("fork: ");
      exit(EXIT_FAILURE);
   }
   else if(pid>0)
   {
      //write(STDOUT_FILENO,"Parent of run fork\n",strlen("Parent of run fork\n"));
      close(pipefd3[1]); //closing write
      char buff5[12];
      int read_count5 = read(pipefd3[0],buff5,6);
      if(read_count5==-1)
      {
         perror("pipe: ");
      }
      else if (0==read_count5)
      {
         pointer++;
         strcpy(process_list[pointer].name,args[0]);
         process_list[pointer].pid = pid;
         process_list[pointer].status=1;
         process_list[pointer].start_time = time(NULL);
         process_list[pointer].end_time = 99;
         process_list[pointer].elapsed_time = 0;
         write(fd,"Exec success\n\0",strlen("Exec success\n\0"));
      }
      else if(0==strcmp(buff5,"mamoo"))
      {
         write(fd,"Exec failed\n\0",strlen("Exec failed\n\0"));
      }

   }
   else if(pid==0)
   {
      //write(STDOUT_FILENO,"Child of run fork\n",strlen("Child of run fork\n"));
      close(pipefd3[0]); //closing read
      int ret = execvp(args[0], args);
      if(ret==-1)
      {
         //perror("execvp: ");
         write(pipefd3[1],"mamoo\0",6);
         exit(EXIT_SUCCESS);
      }
   }
}

static void display_list(char* token, int fd)
{
   struct tm* time_struct;

   token = strtok(NULL, " ,\n");
   if(token == NULL)
   {
      int i=1;
      char temp_list[999];
      int count=0;
      while(i<=pointer)
      {
         if(process_list[i].status == 1)
         {
            process_list[i].elapsed_time = time(NULL) - process_list[i].start_time;

            time_struct = localtime(&(process_list[i].start_time));
            int hour = time_struct->tm_hour;
            int min = time_struct->tm_min;
            int sec = time_struct->tm_sec;

            time_struct = localtime(&(process_list[i].elapsed_time));
            int hour2 = time_struct->tm_hour;
            int min2 = time_struct->tm_min;
            int sec2 = time_struct->tm_sec;

            count += sprintf(&temp_list[count], "Process %d: name: %s pid: %d status: %d start time: %d:%d:%d elapsed time: %d:%d:%d\n", i,process_list[i].name, process_list[i].pid, process_list[i].status, hour,min,sec,hour2,min2,sec2);
         }
         i++;
      }
      temp_list[count]= '\0';
      write(fd, temp_list, count);
   }
   else if(0 == strcmp(token, "all"))
   {
      int i=1;
      char temp_list[100];
      int count=0;
      while(i<=pointer)
      {
         process_list[i].elapsed_time = time(NULL) - process_list[i].start_time;

         time_struct = localtime(&(process_list[i].start_time));
         int hour = time_struct->tm_hour;
         int min = time_struct->tm_min;
         int sec = time_struct->tm_sec;

         time_struct = localtime(&(process_list[i].elapsed_time));
         int hour2 = time_struct->tm_hour;
         int min2 = time_struct->tm_min;
         int sec2 = time_struct->tm_sec;

         if(process_list[i].end_time!=99)
         {
            time_struct = localtime(&(process_list[i].end_time));
            int hour1 = time_struct->tm_hour;
            int min1 = time_struct->tm_min;
            int sec1 = time_struct->tm_sec;

            count += sprintf(&temp_list[count], "Process %d: name: %s pid: %d status: %d start time: %d:%d:%d end time: %d:%d:%d elapsed time: %d:%d:%d\n", i,process_list[i].name, process_list[i].pid, process_list[i].status, hour,min,sec,hour1,min1,sec1,hour2,min2,sec2);
         }

         else
         {
            count += sprintf(&temp_list[count], "Process %d: name: %s pid: %d status: %d start time: %d:%d:%d elapsed time: %d:%d:%d\n", i,process_list[i].name, process_list[i].pid, process_list[i].status, hour,min,sec,hour2,min2,sec2);
         }
         i++;
      }
      temp_list[count]= '\0';
      write(fd, temp_list, count);
   }
   else
   {
      write(fd, "list command not given properly\n\0", 32);
   }
}

static void kill_process(char* token, int fd)
{
   token = strtok(NULL, " ,\n");
   if(token == NULL)
   {
      write(fd,"kill unsuccessful\n\0",strlen("kill unsuccessful\n\0"));
   }
   else if(0==strcmp("all",token))
   {
      for(int i=1; i<=pointer; i++)
      {
         int pid_to_kill=process_list[i].pid;
         kill(pid_to_kill,SIGTERM);
      }
      write(fd,"kill successful\n\0",strlen("kill successful\n\0"));
   }
   else if(isdigit(*token))
   {
      int to_kill = (int)atoi(token);
      int is_kill = 0;
      for(int i=1; i<=pointer; i++)
      {
         if(process_list[i].pid==to_kill)
         {
            is_kill=1;
            break;
         }
      }
      if(is_kill==1)
      {
         int k_err = kill(to_kill, SIGTERM);
         if(k_err==-1)
         {
            perror("kill: ");
         }
         if(k_err==0)
         {
            write(fd,"kill successful\n\0",strlen("kill successful\n\0"));
         }
      }
      else if(is_kill==0)
      {
         write(fd,"kill unsuccessful\n\0",strlen("kill unsuccessful\n\0"));
      }
   }
   else
   {
      int is_kill = 0;
      int pid_to_kill;
      for(int i=1; i<=pointer; i++)
      {
         if((0==strcmp(process_list[i].name,token))&&process_list[i].status==1)
         {
            is_kill=1;
            pid_to_kill = process_list[i].pid;
            break;
         }
      }
      if(is_kill==1)
      {
         int k_err = kill(pid_to_kill, SIGTERM);
         if(k_err==-1)
         {
            perror("kill: ");
         }
         if(k_err==0)
         {
            write(fd,"kill successful\n\0",strlen("kill successful\n\0"));
         }
      }
      else if(is_kill==0)
      {
         write(fd,"kill unsuccessful\n\0",strlen("kill unsuccessful\n\0"));
      }
   }
}

int main(void)
{

   int sock, length;
   struct sockaddr_in server;
   struct sockaddr_in client;
   int msgsock;
   char buf[1024];
   int rval;

   /* Create socket */
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0)
   {
      perror("opening stream socket");
      exit(1);
   }
   /* Name socket using wildcards */
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port = 0;
   if (bind(sock, (struct sockaddr *) &server, sizeof(server)))
   {
      perror("binding stream socket");
      exit(1);
   }
   /* Find out assigned port number and print it out */
   length = sizeof(server);
   if (getsockname(sock, (struct sockaddr *) &server, (socklen_t*) &length))
   {
      perror("getting socket name");
      exit(1);
   }
   printf("Socket has ip #%s\n", inet_ntoa(server.sin_addr));
   printf("Socket has port #%d\n", ntohs(server.sin_port));
   fflush(stdout);

   /* Start accepting connections */
   listen(sock, 5);

   char from_client[999];
   char to_client[999];

   while(1)
   {
      msgsock = accept(sock, (struct sockaddr*) &client, &length);
      if (msgsock == -1)
         perror("accept");

      //printf("Socket has ip #%s\n", inet_ntoa(server.sin_addr));

      int pid =fork();
      if(pid==-1)
      {
         perror("Fork: ");
         exit(EXIT_FAILURE);
      }

      else if(pid==0)
      {

         while(1)
         {
            //sleep(15); to check interactivity
            int read_count1=read(msgsock, from_client,999);
            if(read_count1==-1)
            {
               perror("read: ");
               exit(EXIT_FAILURE);
            }

            else if(read_count1==1)
            {
               write(msgsock,"please type *help* to use the program\n\0",
                     strlen("please type *help* to use the program\n\0"));
               continue;
            }

            //to print command on server
            write(STDOUT_FILENO,from_client,strlen(from_client));
            write(STDOUT_FILENO,"\n",1);

            char* token = strtok(from_client," ,\n");

            if(token == NULL)
            {
               write(msgsock,"please type *help* to use the program\n\0",
                     strlen("please type *help* to use the program\n\0"));
               continue;
            }

            else if(0 == strcmp(token, "add"))
            {
               math(token,msgsock);
            }

            else if(0 == strcmp(token, "sub"))
            {
               math(token,msgsock);
            }

            else if(0 == strcmp(token, "mul"))
            {
               math(token,msgsock);
            }

            else if(0 == strcmp(token, "div"))
            {
               math(token,msgsock);
            }

            else if(0 == strcmp(token, "run"))
            {
               run(token,msgsock);
            }

            else if(0 == strcmp(token, "help"))
            {
               write(msgsock,"this is the help function\nto use this program please enter any of the following functions and list the arguments\nadd num1 num2 ...\nsub num1 num2 ...\nmul num1 num2 ...\ndiv num1 num2 ...\nrun program_name file for the program to open\nlist (this will list the processes in run)\nexit (to terminate the program)\n\0",strlen("this is the help function\nto use this program please enter any of the following functions and list the arguments\nadd num1 num2 ...\nsub num1 num2 ...\nmul num1 num2 ...\ndiv num1 num2 ...\nrun program_name file for the program to open\nlist (this will list the processes in run)\nexit (to terminate the program)\n\0"));
            }

            else if(0 == strcmp(token, "list"))
            {
               display_list(token,msgsock);
            }

            else if(0 == strcmp(token, "exit"))
            {
               write(msgsock,"EXIT\0",5);
               close(msgsock);
               exit(EXIT_SUCCESS);
            }

            else if(0 == strcmp(token, "disconn"))
            {
               write(msgsock,"disconn\0",8);
               close(msgsock);
               break;
            }

            else if(0 == strcmp(token, "kill"))
            {
               kill_process(token,msgsock);
            }

            else
            {
               write(msgsock,"please type *help* to use the program\n\0",
                     strlen("please type *help* to use the program\n\0"));
               continue;
            }
         }
      }
   }
}
