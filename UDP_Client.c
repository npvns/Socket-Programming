#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>

/* port where server is listening for incoming request */
#define PORT "9999"   

/* client program : it accept one argument and that is server ip to connect */
int main(int argc, char* argv[])
{
   struct addrinfo hints, *target_addr, *temp_addr; 
   int rlen = 0, slen = 0, ret = 0, target_fd;
   char buf[100];

   if(argc != 3) { fprintf(stderr, "usage: TALKER HOSTNAME MESSAGE\n"); exit(1);} 
   
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_UNSPEC; /* ipv4 or ipv6 */
   hints.ai_socktype = SOCK_DGRAM; /* stream socket */

   ret = getaddrinfo(argv[1],  /* server ip address, passed as argument*/
                     PORT,     /* port where server listens */
					 &hints,   
					 &target_addr);  /* get sockaddr filled as per destination ip,port */
   if(0 != ret) { fprintf(stderr, "getaddrinfo : %s",gai_strerror(ret)); return 1;}

   /* Create a socket fd for designated family and protocol */
   for(temp_addr = target_addr; temp_addr != NULL; temp_addr = temp_addr->ai_next)
   {
       target_fd = socket(target_addr->ai_family, target_addr->ai_socktype, target_addr->ai_protocol);
       if(-1 == target_fd) { perror("client : socket"); continue;}
       break;
   }
   if(temp_addr == NULL) { fprintf(stderr, "client: failed to create socket\n"); }
   
   slen = sendto(target_fd,
                 argv[2],
				 strlen(argv[2]),
				 0,
				 temp_addr->ai_addr,
				 temp_addr->ai_addrlen);
   if(slen == -1) { perror("client: sendto"); exit(1); }
    
	printf("client sned %d bytes to %s\n", slen, argv[1]); 
   
   freeaddrinfo(target_addr);   
   close(target_fd);
   return 0;
}
  

