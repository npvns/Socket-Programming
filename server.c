#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>

#define PORT "9999"   /* port where server will run and client will connect to */

/* main server programm */
int main()
{
   /* server_addr will hold the server address, port,family, socket type,
      communication protocol */	
   struct addrinfo hints, *server_addr, *temp_addr; 
   
   /* client_addr will hold the connecting clients address,port,family
      information. sockaddr can handle both sockaddr_in(ipv4) and
	  sockaddr_in6(ipv6) */
   struct sockaddr client_addr;  
   
   int client_count = 0;
  
   memset(&hints, 0 , sizeof hints);
   memset(&client_addr, 0, sizeof hints);

   int ret = 0, client_addr_len = 0, rlen = 0, slen = 0, mainfd = 0, newfd = 0;
   char buf[100];

    /* This is only going to listen, can not send connect request */ 
   hints.ai_flags = AI_PASSIVE;     

   /* stream socket, guaranteed delivery of correct packets in right order */   
   hints.ai_socktype = SOCK_STREAM;   
   
   /* AF_UNSPEC: dont care about ipv4 or ipv6. AF_INET : ipv4, AF_INET6 : ipv6 */
   hints.ai_family = AF_UNSPEC;               

   /* get all the sockaddr elements, we don't need to fill it manually.*/
   ret = getaddrinfo(NULL,    /* IP where client will connect */
                     PORT,    /* port where server will run */
					 &hints,  /* Tell that it's going to bea server, socket type and protocol family */ 
					 &server_addr);  /* out parameter */
					 
   if(0 != ret) { fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(ret)); return 1;}

   /* Create a socket fd for designated family and protocol */
   mainfd = socket(server_addr->ai_family, server_addr->ai_socktype, server_addr->ai_protocol);
   if(-1 == mainfd) { perror("server : socket"); return 1;}

   /* Associate socke fd with some port on local machine where server will run.
      This we do if we are gooing to listen for incoming connection on specific
	  port */ 
   ret    = bind(mainfd, server_addr->ai_addr, server_addr->ai_addrlen); 
   if(-1 == ret) { perror("server : bind"); return 1;}
   free(server_addr);
   
    /* Queuing the incoming connect request on mainfd upto 10 clients. */
   ret = listen(mainfd, 10);
   if(-1 == ret) { perror("server : listen"); return 1;}
   
   while(1)
   {
	  /* accpet the new connection request queued up while listen. It returns
         a brand new fd, This new one will server to client connecting and 
         old one is again wait to accept the new connection */ 
      newfd = accept(mainfd, &client_addr, &client_addr_len);
      printf("connection id = %d\n",newfd);
      if(-1 == newfd) { perror("server : accept"); return 1; }
      ret = 0;
      ret = fork(); /* create a new process to serve the client request */
      if(ret == 0)  /* child process */  
      {
        close(mainfd);  /* not required by child process, hence closing it */
        while(1)
        {
		   /* block here to receive some message from client */	
          rlen = recv(newfd, buf, 100, 0);
		  /* if "end" came form client then close the child process taking care of the that client*/
          printf("message from client: %s",buf); 
          if(0 == strncmp(buf,"end",3))
          {
            close(newfd);
            exit(0);
          }
          printf("enter message for client: "); /* send message to client */
          fgets(buf, 100, stdin);
          send(newfd, buf, 100, 0);
        }
     }
   //  else { close(newfd); }
   }
}
   
