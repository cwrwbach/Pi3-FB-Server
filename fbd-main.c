#include <linux/kd.h>
#include <inttypes.h>
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
#include <math.h>
#include "fbd-lib.h"
#include "fbd-colours.h"

#define PORT_1 11361
#define TRACE_LEN 1024
#define HEADER_LEN 16
#define MAX_PAK_LEN TRACE_LEN+HEADER_LEN
#define FRAME_BUF_HEIGHT 600 //768 (ToGuard monitor)
#define CHAN_HEIGHT_A 125
#define CHAN_HEIGHT_B 125
#define CHAN_HEIGHT_C 125
#define CHAN_HEIGHT_D 125
#define PHASE_HEIGHT 125
#define MAX_Y_VAL 120
#define CHAN_SPACE 130
#define CHAN_POS_A 0
#define CHAN_POS_B CHAN_POS_A+CHAN_SPACE+5
#define CHAN_POS_C CHAN_POS_B+CHAN_SPACE+5
#define CHAN_POS_D CHAN_POS_C+CHAN_SPACE+5
#define PHASE_POS CHAN_POS_D
#define RAD_BAM (double)128/(M_PI)

//Main                      Sub 
//A	+ + − − + − + − +	    + + + + + − − +
//B	+ − − + + + + + −	    + − + − + + − −


//Screen data
int fbfd;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
uint g_screen_size_x;
uint screen_size_y;
uint bytes_pp;

//Global buffers
uint16_t * frame_buf;
uint16_t * chan_buf_a;
uint16_t * chan_buf_b;
uint16_t * chan_buf_c;
uint16_t * chan_buf_d;
uint16_t * wfall_buf;
uint16_t * cmd_buf;
uint16_t * fscale_buf;
int8_t * phase_buf;

//Prototypes
int do_network_setup(void);
void draw_spectrum(short);
void draw_waterfall();

uint8_t trace_a[1024];
uint8_t trace_b[1024];
uint8_t trace_c[1024];
uint8_t trace_d[1024];

//Sockets
struct sockaddr_in servaddr_1, cliaddr_1;
int sockfd_1;
socklen_t cliLen_1;
int8_t rx_msg_buffer[MAX_PAK_LEN];

//================

void draw_grid(uint16_t * buf)
{
int i;
int n_horiz;
int h_gap;

n_horiz=4;
h_gap = CHAN_HEIGHT_A/(n_horiz);

//fill backround of SPEC
for(int b=0;b<CHAN_HEIGHT_A * g_screen_size_x;b++)
    buf[b] = rgb565(0,30,0);

for(i=1;i<n_horiz;i++)
    { 
    plot_dotted_line(buf,0,i*h_gap,g_screen_size_x,i*h_gap,C_GREEN_YELLOW);
    }
plot_dotted_line(buf,100,0,100,CHAN_HEIGHT_A-10,C_GREEN_YELLOW);
plot_dotted_line(buf,500,0,500,CHAN_HEIGHT_A-10,C_GREEN_YELLOW);
}

//-----

void draw_freq_scale()  //not called 7 Feb 26
{
char f_left[32];
char f_centre[32];
char f_right[32];

for(int b=0;b<CHAN_HEIGHT_A * g_screen_size_x;b++)
    fscale_buf[b] = 0x8000;

sprintf(f_left," Left");
sprintf(f_right,"Right ");

plot_large_string(fscale_buf,5,20,f_left,C_WHITE);
plot_large_string(fscale_buf,(g_screen_size_x/2) - 50,20,f_centre,C_WHITE);
plot_large_string(fscale_buf,g_screen_size_x -100,20,f_right,C_WHITE);
}

//-----


void draw_phase(uint16_t * buf, int y_pos,int y_size,char * p_buf ,short colour)
{
uint16_t px,py;
float x,y,len,phs;
float from_x,from_y;
char temp;

double ibad = 1/(256 * M_PI);
len = 40.0;

for(int b=0;b<PHASE_HEIGHT * g_screen_size_x;b++)
    buf[b] = rgb565(0,0,24);

for(int n=0;n<8;n++)
    {
    plot_circle(buf,n*100 + 104,60,40,colour);

    from_x=104 + n*100;
    from_y = 60;

    temp = p_buf[n] ;
   // temp &=0x7f;

    phs = (float) temp; // p_buf[n];

    phs /= RAD_BAM ;

    x = (from_x + len * cos(phs));
    y = from_y + len * sin(phs);

    px = (int) x ;
    py = (int) y ;

    plot_line(buf,(int) from_x,(int) from_y,px,py,colour);

    }

copy_surface_to_framebuf(buf,0,y_pos,g_screen_size_x,y_size);
}

//---

void draw_trace(uint16_t * buf, int y_pos,int y_size,char * vid_data ,short colour)
{
int dummy;
char y_val;
char hold;

draw_grid(buf);

for(int p=0;p<1024;p++)
    {
    y_val = vid_data[p];
    if(y_val > MAX_Y_VAL) 
        y_val = MAX_Y_VAL;
    y_val = MAX_Y_VAL - y_val; //invert trace - origin bottom left.
    plot_line(buf,p,hold , p,y_val,colour);
    hold = y_val;
    }

//ioctl(fbfd, FBIO_WAITFORVSYNC, &dummy); // Wait for frame sync
copy_surface_to_framebuf(buf,0,y_pos,g_screen_size_x,y_size);
}

//========================

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

//===================================================

int main()
{
int pak_len;

system("clear");

// Open the framebuffer device file for reading and writing
fbfd = open("/dev/fb0", O_RDWR); 
if (fbfd == -1) 
    printf("Error: cannot open framebuffer device.\n");
 
if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) // Get variable screen information
	    printf("Error reading variable screen info.\n");
printf("Display info %dx%d, %d bpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );

g_screen_size_x = vinfo.xres;
screen_size_y = vinfo.yres;
bytes_pp = vinfo.bits_per_pixel/8;

int fb_data_size = g_screen_size_x * screen_size_y * bytes_pp;
printf (" FB data size = %d \n",fb_data_size);

chan_buf_a = malloc(g_screen_size_x*CHAN_HEIGHT_A*bytes_pp);
chan_buf_b = malloc(g_screen_size_x*CHAN_HEIGHT_B*bytes_pp);
chan_buf_c = malloc(g_screen_size_x*CHAN_HEIGHT_C*bytes_pp);
chan_buf_d = malloc(g_screen_size_x*CHAN_HEIGHT_D*bytes_pp);

wfall_buf = malloc(g_screen_size_x*WFALL_HEIGHT*bytes_pp);
phase_buf = malloc(g_screen_size_x*PHASE_HEIGHT*bytes_pp);
//cmd_buf = malloc(g_screen_size_x*CMD_HEIGHT*bytes_pp);
//fscale_buf = malloc(g_screen_size_x*SCALE_HEIGHT*bytes_pp);

// map framebuffer to user memory 
frame_buf = (uint16_t * ) mmap(0, fb_data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
clear_screen(rgb565(0,0,0));
printf(" SETUP ==========================  \n");

//make Spectrum frame
plot_thick_line(frame_buf,10,CHAN_POS_A+CHAN_SPACE,g_screen_size_x-10,CHAN_POS_A+CHAN_SPACE,C_BLUE);
plot_thick_line(frame_buf,10,CHAN_POS_B+CHAN_SPACE,g_screen_size_x-10,CHAN_POS_B+CHAN_SPACE,C_BLUE);
plot_thick_line(frame_buf,10,CHAN_POS_C+CHAN_SPACE,g_screen_size_x-10,CHAN_POS_C+CHAN_SPACE,C_BLUE);
plot_thick_line(frame_buf,10,CHAN_POS_D+CHAN_SPACE,g_screen_size_x-10,CHAN_POS_D+CHAN_SPACE,C_BLUE);

do_network_setup();

printf(" END NW SETUP \n");

printf(" Bound, waiting for incoming \n");
while(1)
	{
    pak_len  = recvfrom(sockfd_1, & rx_msg_buffer, 1024, 0, ( struct sockaddr *) &cliaddr_1, 
                &cliLen_1); 
     if(pak_len > MAX_PAK_LEN)
        {
        pak_len = MAX_PAK_LEN;
        printf(" RECD EXCESS PAK LEN ! %d \n",pak_len);
        }

if(rx_msg_buffer[1] == 0x66)
    draw_trace(chan_buf_a,CHAN_POS_A,CHAN_HEIGHT_A, rx_msg_buffer+HEADER_LEN, C_RED);
if(rx_msg_buffer[1] == 0x67)
    draw_trace(chan_buf_b,CHAN_POS_B,CHAN_HEIGHT_B, rx_msg_buffer+HEADER_LEN, C_CYAN);

if(rx_msg_buffer[1] == 0x68)
    draw_trace(chan_buf_c,CHAN_POS_C,CHAN_HEIGHT_C, rx_msg_buffer+HEADER_LEN, C_MAGENTA);

//This replaces bottom trace with phase display
if(rx_msg_buffer[1] == 0x69)
  // draw_trace(chan_buf_d,CHAN_POS_D,CHAN_HEIGHT_D, rx_msg_buffer+HEADER_LEN, C_YELLOW);
    draw_phase(phase_buf,PHASE_POS,PHASE_HEIGHT,rx_msg_buffer+HEADER_LEN,C_GREEN);
	}
printf(" DONE \n");
}
