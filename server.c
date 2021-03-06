#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
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
#include <sys/poll.h>
#include <sys/time.h>

struct list
{
   char name[30];
   int pid;
   int status;
   time_t start_time;
   time_t end_time;
   time_t elapsed_time;
};

struct list2
{
   char ip[30];
   int port;
   int client_pid;
   int status;
   int readfd[2];
   int writefd[2];
};

static struct list process_list[30];
static int pointer=0;

static struct list2 client_list[30];
static int no_of_clients=0;

static void sig_handler(int signo)
{
   int status_wait;
   int child_pid = waitpid(-1,&status_wait,WNOHANG);
   if(signo==SIGCHLD)
   {
      for(int i=1; i<=pointer; i++)
      {
         if(process_list[i].pid==child_pid)
         {
            process_list[i].status = 0;
            process_list[i].end_time = time(NULL);
         }
      }
   }
}

static void sig_handler_client(int signo)
{
   int status_wait;
   int child_pid = waitpid(-1,&status_wait,WNOHANG);
   if(signo==SIGCHLD)
   {
      for(int i=1; i<=no_of_clients; i++)
      {
         if(client_list[i].client_pid==child_pid)
         {
            client_list[i].status = 0;
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
      if(token == NULL)
      {
         write(fd, "please enter numbers to add", strlen("please enter numbers to add"));
      }
      else
      {
         while(token != NULL)
         {
            int char_to_int = (int)atoi(token);
            sum = sum + char_to_int;
            token = strtok(NULL," ,\n");
         }
         int sprintf_count = sprintf(answer, "result of add is= %d\n", sum);
         write(fd, answer, sprintf_count);
      }
   }
   else if(0 == strcmp(token, "sub"))
   {
      char answer[100];
      int sum=0;
      token = strtok(NULL, " ,\n");
      if(token == NULL)
      {
         write(fd, "please enter numbers to sub", strlen("please enter numbers to sub"));
      }
      else
      {
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
   }
   else if(0 == strcmp(token, "mul"))
   {
      char answer[100];
      float sum = 1;
      token = strtok(NULL, " ,\n");
      if(token == NULL)
      {
         write(fd, "please enter numbers to mul", strlen("please enter numbers to mul"));
      }
      else
      {
         while(token != NULL)
         {
            float char_to_float = (int)atoi(token);
            sum = sum * char_to_float;
            token = strtok(NULL," ,\n");
         }
         int sprintf_count = sprintf(answer, "result of multiply is= %.3f\n", sum);
         write(fd, answer, sprintf_count);
      }
   }
   else if(0 == strcmp(token, "div"))
   {
      char answer[100];
      float sum=0;
      token = strtok(NULL, " ,\n");
      if(token == NULL)
      {
         write(fd, "please enter numbers to div", strlen("please enter numbers to div"));
      }
      else
      {
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
}

static void run(char* token, int fd)
{
   char* args[30];
   int c=0;
   token = strtok(NULL, " ,\n");
   if(token == NULL)
   {
      write(fd, "please enter program to run", strlen("please enter program to run"));
   }
   else
   {
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
         exit(EXIT_FAILURE);
      }

      int fcntl_err = fcntl(pipefd3[1], F_SETFD, FD_CLOEXEC);
      if(fcntl_err==-1)
      {
         perror("fcntl");
         exit(EXIT_FAILURE);
      }

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
            errno = ret;
            perror("execvp: ");
            write(pipefd3[1],"mamoo\0",6);
            exit(EXIT_SUCCESS);
         }
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
      char temp_list[999];
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

static void kill_all(void)
{
   for(int i=1; i<=pointer; i++)
   {
      int pid_to_kill=process_list[i].pid;
      int kill_return = kill(pid_to_kill,SIGTERM);
      if(kill_return == 0)
      {
         process_list[i].status = 0;
      }
   }
}

static void kill_process(char* token, int fd)
{
   token = strtok(NULL, " ,\n");
   if(token == NULL)
   {
      write(fd,"please enter an argument for kill command\n\0",strlen("please enter an argument for kill command\n\0"));
   }
   else if(0==strcmp("all",token))
   {
      for(int i=1; i<=pointer; i++)
      {
         int pid_to_kill=process_list[i].pid;
         int kill_return2 = kill(pid_to_kill,SIGTERM);
         if(kill_return2 == 0)
         {
            process_list[i].status = 0;
         }
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
      //kill by name
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

static int make_socket_server()
{

   int sock, length;
   struct sockaddr_in server;

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

   return sock;
}

static void display_list_client(char* token, int fd)
{
   token = strtok(NULL, " ,\n");
   if(token == NULL)
   {
      write(fd, "please write command again with arguments\n\0", strlen("please write command again with arguments\n\0"));
   }
   else if(0 == strcmp(token, "clients"))
   {
      int i=1;
      char temp_list[999];
      int count=0;
      while(i<=no_of_clients)
      {
         count += sprintf(&temp_list[count], "Client %d: ip: %s port: %d status: %d pid: %d\n", i,client_list[i].ip, client_list[i].port, client_list[i].status, client_list[i].client_pid);
         i++;
      }
      temp_list[count]= '\0';
      write(fd, temp_list, count);
   }
   else if(0 == strcmp(token, "processes"))
   {
      int i=1;
      char temp_list[5000];
      char buff12[999];
      int count=0;
      while(i<=no_of_clients)
      {
         if(client_list[i].status==1)
         {
            count += sprintf(&temp_list[count], "Client %d: ip: %s port: %d status: %d pid: %d\n", i,client_list[i].ip, client_list[i].port, client_list[i].status, client_list[i].client_pid);
            write(client_list[i].writefd[1],"list all\n",9);
            int read_count70 = read(client_list[i].readfd[0],buff12,999);
            count+=(strlen(strncat(&temp_list[count],buff12,read_count70)));
         }
         i++;
      }
      temp_list[count]= '\0';
      write(fd, temp_list, count);
   }
   else
   {
      write(fd, "list command not given properly\n\0", strlen("list command not given properly\n\0"));
   }
}

int main(void)
{
   int main_sock;
   main_sock=make_socket_server();

   int msgsock;
   struct sockaddr_in client;
   int len = sizeof(client);

   /* Start accepting connections */
   int listen_ret = listen(main_sock, 5);
   if(listen_ret == -1)
   {
      perror("listen");
   }
   else if(listen_ret == 0)
   {
      write(STDOUT_FILENO,"listening...!\n",strlen("listening...!\n"));
   }

   __sighandler_t prev_SIGCHLD_client = signal(SIGCHLD,sig_handler_client);
   if(*prev_SIGCHLD_client==SIG_ERR)
   {
      fprintf(stderr,"Cannot handle SIGCHLD!!!!!\n");
      exit(EXIT_FAILURE);
   }

   struct pollfd fds[2];
   //memset(fds, 0, sizeof(fds));
   int poll_return;

   /*for accepting connections*/
   fds[0].fd = main_sock;
   fds[0].events = POLLIN;

   /*for server commands*/
   fds[1].fd = STDIN_FILENO;
   fds[1].events = POLLIN;

   while(1)
   {

      poll_return = poll(fds, 2, -1);

      if (poll_return == -1)
      {
         if(errno == EINTR)
         {
            continue;
         }
         else
         {
            perror ("poll");
            continue;
         }
      }

      if (poll_return == 0)
      {
         write(STDOUT_FILENO,"timeout!\n",strlen("timeout!\n"));
      }

      if (fds[0].revents & POLLIN)
      {

         write(STDOUT_FILENO,"from socket!\n",strlen("from socket!\n"));

         msgsock = accept(main_sock, (struct sockaddr*) &client, &len);
         if (msgsock == -1)
         {
            perror("accept");
            continue;
         }

         no_of_clients = no_of_clients +1;
         strcpy(client_list[no_of_clients].ip,inet_ntoa(client.sin_addr));
         client_list[no_of_clients].port = ntohs(client.sin_port);
         client_list[no_of_clients].status=1;
         pipe(client_list[no_of_clients].readfd);
         pipe(client_list[no_of_clients].writefd);

         char buf2[100];
         int count7 = sprintf(buf2,"Connected client has ip: %s port: %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
         write(STDOUT_FILENO,buf2,count7);

         int pid =fork();
         if(pid==-1)
         {
            perror("Fork accept");
            continue;
         }

         if(pid>0)
         {
            client_list[no_of_clients].client_pid = pid;
         }

         else if(pid==0)
         {
            close(client_list[no_of_clients].readfd[0]);
            close(client_list[no_of_clients].writefd[1]);

            struct pollfd fds_server_child[2];
            //memset(fds, 0, sizeof(fds));
            int poll_return2;

            /*for client interaction*/
            fds_server_child[0].fd = msgsock;
            fds_server_child[0].events = POLLIN;

            /*pipe between server and child*/
            fds_server_child[1].fd = client_list[no_of_clients].writefd[0];
            fds_server_child[1].events = POLLIN;

            while(1)
            {

               poll_return2 = poll(fds_server_child, 2, -1);

               if (poll_return2 == -1)
               {
                  if(errno == EINTR)
                  {
                     continue;
                  }
                  else
                  {
                     perror ("poll server child");
                     continue;
                  }
               }

               if (poll_return2 == 0)
               {
                  write(STDOUT_FILENO,"timeout!\n",strlen("timeout!\n"));
               }

               if(fds_server_child[0].revents & POLLIN)
               {

                  char from_client[999];
                  char to_client[999];

                  //sleep(30); //to check interactivity
                  int read_count1=read(msgsock, from_client,999);
                  if(read_count1==-1)
                  {
                     perror("read from client");
                     continue;
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
                     write(msgsock,"this is the help function\nto use this program please enter any of the following functions and list the arguments\n\ndisconn\n\nadd num1 num2 num3 ...\nsub num1 num2 num3 ...\nmul num1 num2 num3 ...\ndiv num1 num2 num3 ...\n\nhelp\n\nrun [process name] [file to open by the process]\nlist | [ all ]\n\nkill [ pid | process name | all ]\n\nexit\n\0",strlen("this is the help function\nto use this program please enter any of the following functions and list the arguments\ndisconn\n\nadd num1 num2 num3 ...\nsub num1 num2 num3 ...\nmul num1 num2 num3 ...\ndiv num1 num2 num3 ...\n\nhelp\nrun [process name] [file to open by the process]\nlist | [ all ]\n\nkill [ pid | process name | all ]\n\nexit\n\n\0"));
                  }

                  else if(0 == strcmp(token, "list"))
                  {
                     display_list(token,msgsock);
                  }

                  else if(0 == strcmp(token, "exit"))
                  {
                     kill_all();
                     write(msgsock,"EXIT\0",5);
                     close(msgsock);
                     exit(EXIT_SUCCESS);
                  }

                  else if(0 == strcmp(token, "disconn"))
                  {
                     kill_all();
                     write(msgsock,"disconn\0",8);
                     close(msgsock);
                     exit(EXIT_SUCCESS);
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

               if(fds_server_child[1].revents & POLLIN)
               {
                  char buff11[50];
                  int read_count69 = read(client_list[no_of_clients].writefd[0],buff11,50);
                  if(read_count69==-1)
                  {
                     perror("read from server");
                     continue;
                  }
                  if(read_count69==1)
                  {
                     continue;
                  }
                  char* token = strtok(buff11," ,\n");
                  if(token == NULL)
                  {
                     write(client_list[no_of_clients].readfd[1],"please type a command\n\0",strlen("please type a command\n\0"));
                     continue;
                  }
                  else if(0==strcmp(token,"list"))
                  {
                     display_list(token,client_list[no_of_clients].readfd[1]);
                  }
               }
            }
         }
      }

      if (fds[1].revents & POLLIN)
      {
         write(STDOUT_FILENO,"read user input to server\n",strlen("read user input to server\n"));

         char screen_to_server[100];
         int read_count5=read(STDIN_FILENO, screen_to_server,100);
         if(read_count5==-1)
         {
            perror("read: ");
            exit(EXIT_FAILURE);
         }

         if(read_count5==1)
         {
            continue;
         }

         screen_to_server[read_count5-1]='\0';

         char* token2 = strtok(screen_to_server," ,\n");

         if(token2 == NULL)
         {
            continue;
         }

         else if(0 == strcmp(token2, "list"))
         {
            display_list_client(token2,STDOUT_FILENO);
         }
      }
   }
}
