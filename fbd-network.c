#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>

#define PORT_1 11361

//Sockets
struct sockaddr_in servaddr_1, cliaddr_1;
//socklen_t cliLen_1;
int sockfd_1;

//============================

int do_network_setup()
{
    // Creating socket file descriptor 
if ( (sockfd_1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
	printf("Socket creation failed /n"); 
       
// Fill server information 
servaddr_1.sin_family    = AF_INET; // IPv4 
servaddr_1.sin_addr.s_addr = INADDR_ANY; 
servaddr_1.sin_port = htons(PORT_1); 
      
// Bind the socket 
if ( bind(sockfd_1,(const struct sockaddr *)&servaddr_1, sizeof(servaddr_1)) < 0 ) 
    printf("Bind failed\n"); 

return 0;
}
