#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>

#define PORT "9999"   /* port where server will run and client will connect to */

/* Generic(INET/INET6) funtion to return IP address */
void* get_addr(struct sockaddr* sa)
{
   if(sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
   return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* main server programm */
int main()
{
   /* server_addr will hold the server address, port,family, socket type,
      communication protocol */		
   struct addrinfo hints, *server_addr;   
   
    /* client_addr will hold the connecting clients address,port,family
       information. sockaddr can handle both sockaddr_in(ipv4) and
	   sockaddr_in6(ipv6) */  
   struct sockaddr_storage client_addr;   
   
   socklen_t client_addr_len = 0;
   int client_count = 0, maxfd = 0, yes = 1;
   void* addr;

   /* read_fds maintains list of socket descriptor to be observed to be ready
      for read/write */  
   fd_set master_fds, read_fds;           

   char remote_ip[INET6_ADDRSTRLEN];      /* holds client IP */
  
   memset(&hints, 0 , sizeof hints);
   memset(&client_addr, 0, sizeof hints);

   int ret = 0, rlen = 0, slen = 0, mainfd = 0, newfd = 0;
   int i = 0, j = 0;
   char buf[100];

   /* This is only going to listen, can not send connect request */
   hints.ai_flags = AI_PASSIVE;  

  /* stream socket, guaranteed delivery of correct packets in right order */   
   hints.ai_socktype = SOCK_STREAM; 

   /* AF_UNSPEC: dont care about ipv4 or ipv6. AF_INET : ipv4, AF_INET6 : ipv6 */   
   hints.ai_family = AF_ANSPEC;           

   /* get all the sockaddr elements, we don't need to fill it manually.*/   
   ret = getaddrinfo(NULL, /* IP where client will connect */
                     PORT, /* port where server will run */ 
					 &hints, /* Tell that it's going to bea server, socket type and protocol family */
					 &server_addr); /* out parameter */
   if(0 != ret) { fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(ret)); return 1;}

   /* Create a socket fd for designated family and protocol */ 
   mainfd = socket(server_addr->ai_family, server_addr->ai_socktype, server_addr->ai_protocol);
   if(-1 == mainfd) { perror("server : socket"); return 1;}

   /* Enable for reusing the existing port */ 
   setsockopt(mainfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

   /* Associate socke fd with some port on local machine where server will run.
      This we do if we are gooing to listen for incoming connection on specific
	  port */ 
   ret    = bind(mainfd, server_addr->ai_addr, server_addr->ai_addrlen); 
   if(-1 == ret) { perror("server : bind"); return 1;}
   free(server_addr);
   
   /* Queuing the incoming connect request on mainfd upto 10 clients. */
   ret = listen(mainfd, 10);
   if(-1 == ret) { perror("server : listen"); return 1;}

   /* add server fd to descriptor set */
   FD_SET(mainfd, &master_fds);
   maxfd = mainfd;
 
   while(1)
   {
    read_fds = master_fds;
	/* accpet the new connection request queued up while listen. It returns
       a brand new fd, This new one will server to client connecting and 
       old one is again wait to accept the new connection */ 
    ret = select(maxfd+1, &read_fds, NULL, NULL, NULL); 
    if(-1 == ret) { perror("server : select"); return 1; }
    
	/* traver through all the fds */
    for(i = 0; i <= maxfd; i++)
    {
      if(FD_ISSET(i, &read_fds))  /* if this fd is added to descriptor to be monitorred */
      {
		/* server fd is ready to accept new connection. */  
        if(i == mainfd)       
        {  
           client_addr_len = sizeof client_addr; 
		   /* We are here because there is a new connection request, accept is not going to
		      be blocked. This returns a new fd dedicated for communication between client
              and server. This will also return IP nad port of the clients connecting.*/
           newfd = accept(mainfd, (struct sockaddr*)&client_addr, &client_addr_len); 
           if(-1 == newfd) { perror("server : accept"); return 1; }
		   /* a new client conenction came, add it into fd set */
           else { FD_SET(newfd, &master_fds); if(newfd > maxfd) maxfd = newfd;}

		   /* print who is connecting */ 
           addr = get_addr((struct sockaddr*)&client_addr);
		   
		   /* converts ip address into string to be printed */
           char* p = inet_ntop(client_addr.ss_family,
                               addr,
                               remote_ip,
                               client_addr_len);

           printf("server : new connection from %s on socket %d on port %d\n",
                   remote_ip,
                   newfd,
                   ((struct sockaddr_in*)(&client_addr))->sin_port);                              
        }                
        else       /* Here, because some client sent messages for others.*/     
        {
          rlen = recv(i, buf, 100, 0);
          if(0 >= rlen)   
          {
			/* client has closed the connection */  
            if(0 == rlen) { printf("server : socket %d closed up\n",i); }  
			
			/* due to some error client is not approachable */
            else  { perror("server : recv"); }
            close(i); FD_CLR(i, &master_fds);
          }
          else
          {
			/* again loop thorugh all the fds and forward the received message to all
             * other client except server and itself. */			
            for(j = 0; j <= maxfd; j++)
            {
              if(FD_ISSET(j, &master_fds))
              {
                if( (j != mainfd) && (j != i) )
                {
                  slen = send(j, buf, rlen, 0);
                  if(-1 == slen) { perror("server:send"); }
                }
              }
            }
          }
        }
      }
    }
  }
  return 0;
}
      
     
   
