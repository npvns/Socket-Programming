#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>

#define STDIN 0      /* standard input fd */
#define PORT "9999"    /* port where server is running and client will connect to */

/* client program : it accept one argument and that is server ip to connect */
int main(int argc, char* argv[])
{
   struct addrinfo hints, *target_addr;
   int rlen = 0, slen = 0, ret = 0, target_fd;
   char buf[100];
   int i = 0;

   fd_set read_fds, read_fds_copy;
   
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;

   /* input server ip:port along with some hints, target_addr will be filled up */
   ret = getaddrinfo(argv[1],   /* server ip address, passed as argument*/ 
                     PORT,      /* port where server listens */
					 &hints,
					 &target_addr); /* get sockaddr filled as per destination ip,port */
   if(0 != ret) { fprintf(stderr, "getaddrinfo : %s",gai_strerror(ret)); return 1;}

   /* create a socket fd for designated family and protocol */
   target_fd = socket(target_addr->ai_family, target_addr->ai_socktype, target_addr->ai_protocol);
   if(-1 == target_fd) { perror("client : socket"); return 1;}

    /* connect to server ip, port */
   ret = connect(target_fd, target_addr->ai_addr, target_addr->ai_addrlen);
   if(-1 == ret) { perror("client : connect"); return 1;}

   /* STDIN will be observed if something is written to STDIN */
   FD_SET(STDIN, &read_fds);  
   
   /* read_fds will be observed if something came to be read */
   FD_SET(target_fd, &read_fds);
   
   read_fds_copy = read_fds;
   while(1)
   {
    read_fds = read_fds_copy;
    printf("waiting on select\n");
	/* client will block here, it will be unblocked if somethig
       is writted to STDIN or came to target_fd */
    ret = select(target_fd+1, &read_fds, NULL, NULL, NULL);
    if(-1 == ret) { perror("server : select"); return 1; }

    for(i = 0; i <= target_fd; i++)
    {
      if(FD_ISSET(i, &read_fds))   /* go inside if fd is added to set*/
      {
         if(i == STDIN)    /* something written to STDIN, read it */
         {  
		   /* read from stdin and store it into buffer */
           fgets(buf, 100, stdin);
		   
		   /* send this buffer content to server, where server will
		      forward this messages to other clients */
           slen = send(target_fd, buf, 100, 0);
         }
         else if(i == target_fd)  /* something written to targte_fd, read it */
         {
           rlen = recv(target_fd, buf, 100, 0);
           if(0 >= rlen)
           {
			 /* server closed the connection */  
             if(0 == rlen) { printf("server : socket %d closed up\n",i); }  
			 
			 /* some error */
             else  { perror("client : recv"); }
			 
			 /* close the fd and remove it's entry from fd set */
             close(i); FD_CLR(i, &read_fds);
             break;
           }
           /* display the broadcasted message */
           printf("messages broadcasted by some server:%s",buf);
         }
     }
    }
  }
   close(target_fd);
}
  
