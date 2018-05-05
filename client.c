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
#include <pthread.h>
#include <errno.h>

int make_socket(void)
{
   int sock;

   while(1)
   {
      write(STDOUT_FILENO,"to connect to the server please type:\nconn <ip address> <port>\n",strlen("to connect to the server please type:\nconn <ip address> <port>\n"));

      char buf1[100];
      int count1;

      count1= read(STDIN_FILENO,buf1,100);
      if(count1==-1)
      {
         perror("read: ");
         continue;
      }

      buf1[count1]='\0';

      char* token = strtok(buf1," ,\n");

      if(token == NULL)
      {
         write(STDOUT_FILENO,"please enter a valid command\n",strlen("please enter a valid command\n"));
         continue;
      }
      else if(0 == strcmp(token, "conn"))
      {
         char* args[30];
         int c=0;
         token = strtok(NULL, " ,\n");
         if(token == NULL)
         {
            continue;
         }
         while(token != NULL)
         {
            args[c++] = token;
            token = strtok(NULL," ,\n");
         }
         args[c]=NULL;

         struct sockaddr_in server;
         struct hostent *hp;

         /* Create socket */
         sock = socket(AF_INET, SOCK_STREAM, 0);
         if (sock < 0)
         {
            perror("opening stream socket");
            continue;
         }

         /* Connect socket using name specified by command line. */
         server.sin_family = AF_INET;
         hp = gethostbyname(args[0]);
         if (hp == 0)
         {
            fprintf(stderr, "%s: unknown host\n", args[0]);
            continue;
         }

         bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
         server.sin_port = htons(atoi(args[1]));

         if (connect(sock,(struct sockaddr *) &server,sizeof(server)) < 0)
         {
            perror("connecting stream socket");
            continue;
         }
         break;
      }
      else
      {
         write(STDOUT_FILENO,"please enter a valid command\n",strlen("please enter a valid command\n"));
         continue;
      }
   }
   return sock;
}

void* user_to_server(void* ptr)
{
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

   int* sock = (int*) ptr;

   char to_server[999];
   int read_count1;

   write(STDOUT_FILENO,"Please type help to show available commands or just type a command to get started\n",strlen("Please type help to show available commands or just type a command to get started\n"));
   write(STDOUT_FILENO,"-->",strlen("-->"));

   while(1)
   {
      read_count1=read(STDIN_FILENO,to_server,999);
      if(read_count1==-1)
      {
         perror("read: ");
         continue;
      }

      to_server[read_count1-1]='\0';
      write(*sock,to_server, read_count1);
   }
}

int main()
{
   while(1)
   {
      int sock;

      sock = make_socket();

      pthread_t thread1;

      int ret =  pthread_create(&thread1, NULL, user_to_server,(void*) &sock);
      if(ret!=0)
      {
         errno = ret;
         perror("pthread_create: ");
         exit(EXIT_FAILURE);
      }

      char from_server[999];
      int read_count2;

      while(1)
      {
         read_count2=read(sock,from_server,999);
         if(read_count2==-1)
         {
            perror("read: ");
            continue;
         }
         if(strcmp(from_server,"EXIT")==0)
         {
            close(sock);
            exit(EXIT_SUCCESS);
         }
         else if(strcmp(from_server,"disconn")==0)
         {
            close(sock);
            pthread_cancel(thread1);
            break;
         }
         else
         {
            write(STDOUT_FILENO,from_server,read_count2);
            write(STDOUT_FILENO,"-->",strlen("-->"));
         }
      }
   }

   //pthread_join(thread1,NULL);
}
