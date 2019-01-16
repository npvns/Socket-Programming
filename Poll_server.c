#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<poll.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/ioctl.h>

/* port where server will run and client will connect to */
#define PORT "9999"

/* main server programm */
int main()
{
   /* server_addr will hold the server address, port,family, socket type,
      communication protocol */	
   struct addrinfo hints, *server_addr, *temp_addr; 
   int yes = 1, end = 0, close_conn = 0;
   struct pollfd fds[100];
   int nfds = 1, i = 0, j = 0, compress_array = 0;
   /* client_addr will hold the connecting clients address,port,family
      information. sockaddr can handle both sockaddr_in(ipv4) and
      sockaddr_in6(ipv6) */
   struct sockaddr client_addr;  
   
   int current_size = 0;
   int    timeout;
   char buf[10];
   int ret = 0, client_addr_len = 0, rlen = 0, slen = 0, mainfd = 0, newfd = 0;
  
   memset(&hints, 0 , sizeof hints);
   memset(&client_addr, 0, sizeof hints);

   /* This is only going to listen, can not send connect request */ 
   hints.ai_flags = AI_PASSIVE;     

   /* stream socket, guaranteed delivery of correct packets in right order */   
   hints.ai_socktype = SOCK_STREAM;   
   
   /* AF_UNSPEC: dont care about ipv4 or ipv6. AF_INET : ipv4, AF_INET6 : ipv6 */
   hints.ai_family = AF_UNSPEC;               

   /* get all the sockaddr elements, we don't need to fill it manually.*/
   ret = getaddrinfo(NULL,    /* IP where client will connect */
                     PORT,    /* port where server will run */
		     &hints,  /* Tell that it's going to be a server, socket type and protocol family */ 
		     &server_addr);  /* out parameter */
					 
   if(0 != ret) { fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(ret)); return 1;}

   /* Create a socket fd for designated family and protocol */
   mainfd = socket(server_addr->ai_family, server_addr->ai_socktype, server_addr->ai_protocol);
   if(-1 == mainfd) { perror("server : socket"); return 1;}

   /* Allow socket descriptor to be reusabel */
   ret = setsockopt(mainfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
   if(ret == -1) { perror("server:setsockopt"); close(mainfd); return 2; }
	
   /* Make socket non-blocking, all new socket created to serve for incoming
      connection request will also inherit this property */
   //ret = ioctl(mainfd, FIONBIO, &yes);
     ret = fcntl(mainfd, F_SETFL, O_NONBLOCK);
   if(ret == -1) { perror("server:ioctl"); close(mainfd); return 2; }
	
   /* Associate socke fd with some port on local machine where server will run.
      This we do if we are gooing to listen for incoming connection on specific
      port */ 
   ret = bind(mainfd, server_addr->ai_addr, server_addr->ai_addrlen); 
   if(-1 == ret) { perror("server : bind"); return 1;}
   free(server_addr);
   
   /* Queuing the incoming connect request on mainfd upto 10 clients. */
   ret = listen(mainfd, 10);
   if(-1 == ret) { perror("server : listen"); return 1;}
   
   /* Initialize the polfd structure */
   memset(fds, 0, sizeof fds);
	
   fds[0].fd = mainfd;
   fds[0].events = POLLIN;
	
   timeout = 3 * 60 * 1000;
	
   do
   {
    printf("waiting on poll\n");
   /* poll for 3 minutes for connection request */
   ret = poll(fds, nfds, timeout);
	   
   /* if poll has failed */
   if(ret == -1) { perror("server:poll"); break; }
	   
   /* if timeout has expired */
   else if(ret == 0) { perror("server:poll timed out"); break; }
	   
   /* One or more fd are readable, check who are those listening
      or already active connection serveing to some client */
   current_size = nfds;
   for(i = 0; i < current_size; i++)
   {
     if(fds[i].revents == 0) continue;
     else if(fds[i].revents != POLLIN)
     {
       printf("server:error, %d is invalid revent\n",fds[i].revents); 
       end = 1;
       break;
     }
    /* mainfd is readable, some new connection request came */
    if(fds[i].fd == mainfd)
    {
     /* accept all queued request. If accept fails with EWOULDBLOCK, 
        it mains all are accpeted. any other failure means end of                 
        server */	
     do{
	/* accpet the new connection. It returns a brand new fd,
	   This new one will server to client connecting and old
	   one is again wait to accept the new connection */ 
        newfd = accept(mainfd, &client_addr, &client_addr_len);
	if(newfd < 0)
	{ 
          if(errno != EWOULDBLOCK)
	  { 
	    perror("server : accept");
            end = 1;
          }
	  break;
        }
	printf("new incoming connection id = %d\n",newfd);
				
	/* add this ne w fd to poll fd structure */
	fds[nfds].fd = newfd;
	fds[nfds].events = POLLIN;
	nfds++;
      }while(newfd != -1);
    }
    /* this is an existing active readable connection*/		 
    else
    {
      printf("fd %d is readable\n",fds[i].fd);
      close_conn = 0;
     /* receive all incoming data before loopback for polling again */
     do {
	  /* receive data till recv fail with EWOULDBLOCK, if any
           other error close the connection */
          rlen = recv(fds[i].fd, buf, sizeof buf, 0);
          if(rlen < -1)
	  {
             if(errno != EWOULDBLOCK)
             {
                perror("server:recv");
                close_conn = 1;
              }
              break;
          }
          if(rlen == 0) /* client closed the connection */
          {
	     printf("connection %d closed\n",fds[i].fd);
       	     close_conn = 1;
	     break;
          } 			
          /* data received */
           printf("Lenght of received data is %d\n",rlen);

          /* echo bakc the data */
          slen = send(fds[i].fd, buf, rlen, 0);						 
	  if(slen == -1)
	  {
             perror("server:send");
             close_conn = 1;
	     break;
          }
        }while(1); /* receive finish */	
        if(close_conn == 1)
        {
          close(fds[i].fd); fds[i].fd = -1; compress_array = 1;
        }					
      } /* mesage from client is read */			 
    } /* for loop */ 

    if(compress_array == 1)
    {
      compress_array = 0;
      for(i = 0; i < nfds; i++)
      {
        if(fds[i].fd == -1)
        {
          for(j = i; j < nfds; j++)
	  {
	    fds[j].fd = fds[j+1].fd;
	  }
	  i--;
	  nfds--;
	}
      }
    }
   }while(1);
   /*cleanup all the sockets that are open */
   for(i = 0; i < nfds; i++)
   {
      if(fds[i].fd >= 0) close(fds[i].fd); 
   }	   
}
   
