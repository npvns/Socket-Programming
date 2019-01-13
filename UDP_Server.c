#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>

/* port where server will run and client will connect to */
#define PORT "9999"   
#define MAXBUFLEN 100
void* get_cleint_addr(struct sockaddr* sa)
{
   if(sa->sa_family == AF_INET)
   {
      return &(((struct sockaddr_in*)sa)->sin_addr);
   }
   return &(((struct sockaddr_in6*)sa)->sin6_addr);
}   

/* main server programm */
int main()
{
   /* server_addr will hold the server address, port,family, socket type,
    * communication protocol */	
   struct addrinfo hints, *server_addr, *temp_addr; 
   
   /* client_addr will hold the connecting clients address,port,family
    * information. sockaddr can handle both sockaddr_in(ipv4) and
    * sockaddr_in6(ipv6) */
   struct sockaddr_storage client_addr;  
   socklen_t client_addr_len = 0;
   
   char ip[INET6_ADDRSTRLEN];
   
   int client_count = 0;
  
   void* ca = NULL;
   
   memset(&hints, 0 , sizeof hints);
   memset(&client_addr, 0, sizeof hints);

   int ret = 0, rlen = 0, slen = 0, mainfd = 0, newfd = 0;
   char buf[MAXBUFLEN];

    /* This is only going to listen, can not send connect request */ 
   hints.ai_flags = AI_PASSIVE;     

   /* stream socket, guaranteed delivery of correct packets in right order */   
   hints.ai_socktype = SOCK_DGRAM;   
   
   /* AF_UNSPEC: dont care about ipv4 or ipv6. AF_INET : ipv4, AF_INET6 : ipv6 */
   hints.ai_family = AF_UNSPEC;               

   /* get all the sockaddr elements, we don't need to fill it manually.*/
   ret = getaddrinfo(NULL,    /* IP where client will connect */
                     PORT,    /* port where server will run */
					 &hints,  /* Tell that it's going to bea server, socket type and protocol family */ 
					 &server_addr);  /* out parameter */
					 
   if(0 != ret) { fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(ret)); return 1;}

   /*loop through all the nodes and bin with the first one we can */
   for(temp_addr = server_addr; temp_addr != NULL; temp_addr = temp_addr->ai_next)
   {
      /* Create a socket fd for designated family and protocol */
      mainfd = socket(temp_addr->ai_family, temp_addr->ai_socktype, temp_addr->ai_protocol);
      if(-1 == mainfd) { perror("server : socket"); continue;}
	  

     /* Associate socke fd with some port on local machine where server will run.
      * This we do if we are gooing to listen for incoming connection on specific
      * port */ 
     ret    = bind(mainfd, temp_addr->ai_addr, temp_addr->ai_addrlen); 
     if(-1 == ret) { close(mainfd); perror("server : bind"); continue;}
	 break;
   }
   if(temp_addr == NULL) { fprintf(stderr, "server: failed to bind socket\n"); return 2;}
   freeaddrinfo(server_addr);
   
   printf("Listener is waiting to recvfrom...\n");   

   client_addr_len = sizeof client_addr;
      
   /* block here to receive some message from client */	
   rlen = recvfrom(mainfd,
   	               buf,
				   MAXBUFLEN-1,
				   0,
				   (struct sockaddr*)(&client_addr),
				   &client_addr_len);
   buf[rlen] = '\0';				  

   if(rlen == -1) { perror("server:recvfrom"); exit(1); }
   ca = get_cleint_addr( (struct sockaddr*)(&client_addr));
   char* ipadd = inet_ntop(client_addr.ss_family,
	                       ca,
	 					   ip,
                           sizeof ip);
   printf("server got messages from %s\n", ip);	
   printf("packet length is %d and content is %s\n", rlen, buf);
   close(mainfd);	  
}
   

