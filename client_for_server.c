#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>

#define PORT "9999"   /* port where server is listening for incoming request */

/* client program : it accept one argument and that is server ip to connect */
int main(int argc, char* argv[])
{
   struct addrinfo hints, *target_addr; 
   int rlen = 0, slen = 0, ret = 0, target_fd;
   char buf[100];

   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET; /* ipv4 */
   hints.ai_socktype = SOCK_STREAM; /* stream socket */

   ret = getaddrinfo(argv[1],  /* server ip address, passed as argument*/
                     PORT,     /* port where server listens */
					 &hints,   
					 &target_addr);  /* get sockaddr filled as per destination ip,port */
   if(0 != ret) { fprintf(stderr, "getaddrinfo : %s",gai_strerror(ret)); return 1;}

   /* Create a socket fd for designated family and protocol */
   target_fd = socket(target_addr->ai_family, target_addr->ai_socktype, target_addr->ai_protocol);
   if(-1 == target_fd) { perror("client : socket"); return 1;}

   /* connect to server ip, port */ 
   ret = connect(target_fd, target_addr->ai_addr, target_addr->ai_addrlen);
   if(-1 == ret) { perror("client : connect"); return 1;}

   /* Now  coonection is established, start communication */
   while(1)
   {
     printf("enter data for server: ");
     fgets(buf, 100, stdin);
     slen = send(target_fd, buf, 100, 0);
     if(0 == strncmp(buf, "end", 3))
     { 
        break;
     }

     rlen = recv(target_fd, buf, 100, 0);
     printf("messages from server:%s",buf);
  
   }
   close(target_fd);
}
  
