#include <unistd.h>
#include <linux/kd.h>
#include "fbd-colours.h"
#include <stdint.h>
#include <inttypes.h>
#include <linux/kd.h>


#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>


typedef unsigned int uint;

#define FFT_SIZE 1024
#define MAX_IN_BUF 256
#define PORT_1 11361


extern int8_t fbd_buf[FFT_SIZE];
//vws_cnx* cnx;
int watch_dog;

extern uint g_centre_freq;
extern int g_zoom;
extern int g_speed;
extern uint g_url;


void draw_spectrum(short);
void draw_waterfall();

//Sockets
struct sockaddr_in servaddr_1, cliaddr_1;
socklen_t cliLen_1;
int sockfd_1;

//struct sockaddr_in servaddr_1, cliaddr_1;
//socklen_t cliLen_1;
//int sockfd_1;





//============================



int do_network_setup()
{
int debug=0;
int rx_msg_buffer[256];
char respond[256]; 
cliLen_1 = sizeof(struct sockaddr_in);
socklen_t len;
      
// Creating socket file descriptor 
if ( (sockfd_1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
	printf("Socket creation failed /n"); 
  
memset(&cliaddr_1, 0, sizeof(cliaddr_1)); 
      
// Fill server information 
servaddr_1.sin_family    = AF_INET; // IPv4 
servaddr_1.sin_addr.s_addr = INADDR_ANY; 
servaddr_1.sin_port = htons(PORT_1); 
      
// Bind the socket 
if ( bind(sockfd_1,(const struct sockaddr *)&servaddr_1, sizeof(servaddr_1)) < 0 ) 
    printf("Bind failed\n"); 



  

while(0)
{
len = sizeof(cliaddr_1);

printf(" Bound, waiting for incoming tulip\n");
recvfrom(sockfd_1, & rx_msg_buffer, MAX_IN_BUF, 0, ( struct sockaddr *) &cliaddr_1, 
                &len); 
printf(" Got a caller, >> %s \n",rx_msg_buffer);

//draw_trace(chan_buf_a,CHAN_POS_A,CHAN_HEIGHT_A, trace_a, C_RED);

sprintf(respond,"Message from Server %d \n",debug++);
// sending ack \n");

sendto(sockfd_1, (const char *)respond, strlen(respond), 0 , (const struct sockaddr *) &cliaddr_1,len); 

printf("Hello ACK message sent to client.\n");  

}

return 0;
}
