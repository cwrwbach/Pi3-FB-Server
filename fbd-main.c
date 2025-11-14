#include <linux/kd.h>
#include <inttypes.h>


#include "fbd-conf.h"
#include "fbd-lib.h"
#include "fbd-colours.h"
#include "fbd-jet.h"


#define INTERP 0 //to accomodate 1024/1280 pixels width

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
int fbfd;
long int screensize ;
uint g_centre_freq;
int g_zoom;
int g_speed;
uint g_url;
uint g_ntabs;
uint g_tab_width;
uint g_tab_height;
uint g_screen_size_x;

int cmd_select;

int8_t kiwi_buf[FFT_SIZE];


uint screen_size_y;
uint bytes_pp;
uint status_pos;
uint16_t * frame_buf;
uint16_t * spec_buf;
uint16_t * wfall_buf;
uint16_t * cmd_buf;
uint16_t * fscale_buf;

uint8_t qtj[3];
void * setup_kiwi();
pthread_t kiwi_callback;
pthread_t shuttle_callback;
void draw_spectrum(short);
void draw_waterfall();
uint16_t get_colour(int);
void shuttle();

extern int sx_dial;
extern int sx_ring;

int box_width = 275;

extern struct items_status sx_stat; 
//extern vws_cnx* cnx;

extern int btn_0;
extern int btn_1;
extern int btn_2;
extern int btn_3;
extern int btn_4;

#define SYS 0
#define ZOOM 1
#define CF 2
#define SPEED 3
#define URL 4

//================

void draw_grid()
{
int i;
int n_horiz;
int h_gap;

n_horiz=4;
h_gap = SPEC_HEIGHT/(n_horiz);

for(i=1;i<n_horiz;i++){ 
    printf(" i=%d \n",i);
    plot_dotted_line(spec_buf,0,i*h_gap,g_screen_size_x,i*h_gap,C_YELLOW);//YELLOW);
    }
plot_dotted_line(spec_buf,g_screen_size_x/2,0,g_screen_size_x/2,SPEC_HEIGHT,C_YELLOW);//YELLOW);


plot_large_string(spec_buf,900,20," -60",C_WHITE);
plot_large_string(spec_buf,900,60," -80",C_WHITE);
plot_large_string(spec_buf,900,100,"-100",C_WHITE);
plot_large_string(spec_buf,900,140,"-120",C_WHITE);
}

void draw_freq_scale()
{
char f_left[32];
char f_centre[32];
char f_right[32];

for(int b=0;b<SCALE_HEIGHT * g_screen_size_x;b++)
    fscale_buf[b] = 0x8000;

sprintf(f_left," Left");
sprintf(f_centre,"%d",g_centre_freq);
sprintf(f_right,"Right ");

plot_large_string(fscale_buf,5,20,f_left,C_WHITE);
plot_large_string(fscale_buf,(g_screen_size_x/2) - 50,20,f_centre,C_WHITE);
plot_large_string(fscale_buf,g_screen_size_x -100,20,f_right,C_WHITE);

//plot_large_string(fscale_buf,g_centre_freq-10000,20,">|<",C_GREEN); //indicator symbol
copy_surface_to_framebuf(fscale_buf,0,SCALE_POS,g_screen_size_x,SCALE_HEIGHT);
}

void draw_spectrum(short colour)
{
int screen_val;
int16_t level_db;
int spec_base;
int xpos;
unsigned int xtra;
spec_base = SPEC_BASE_LINE;

//fill backround of SPEC
for(int b=0;b<SPEC_HEIGHT * g_screen_size_x;b++)
    spec_buf[b] = C_DARK_GREEN;
//copy_surface_to_framebuf(spec_buf,0,6,g_screen_size_x,SPEC_HEIGHT);
sleep(2);
draw_grid();
xpos = 12; //left hand start pos of spectrum

for(int n = 1; n < FFT_SIZE; n++)

    {
colour = C_RED;

    level_db = n; //= kiwi_buf[n] - 25 + (3*g_zoom); //should be compensated for each Zoom value
    //120 as minimum - gives us db120
    screen_val= 120 + level_db; //positive relatve to -120
    screen_val *=2; //Scale up * 2
    //Guard against over value
    if(screen_val <0) screen_val =0;
    if(screen_val >= SPEC_HEIGHT) screen_val = SPEC_HEIGHT-1;
    //screen origin is bottom left.
  //  plot_line(spec_buf,xpos,spec_base , xpos,spec_base - screen_val,colour); //Plots pos've from bottom left.
    plot_line(spec_buf,0,100 , 400,0,colour);

/*
    if(INTERP ==1) //Interp from 1024 bins to 1280 pixels. i.e 1 extra pixel per 4 drawn.
        {
        if(xtra > 3)
            {
            xtra = 0;
            xpos++;
            plot_line(spec_buf,xpos,spec_base , xpos,spec_base - screen_val,colour); //Plots pos've from bottom left.
            }
        }
*/
    xpos++;
    xtra++;
    }
colour = C_MAGENTA;
 plot_line(spec_buf,0,100 , 900,80,colour);

copy_surface_to_framebuf(spec_buf,0,6,g_screen_size_x,SPEC_HEIGHT);
printf(" FOUND %d \n",__LINE__);

usleep(500000);


}

//=========

void draw_waterfall()
{
uint16_t colour;
int point;
int inx;
int xpos;
unsigned int xtra;
int wfall_width;

wfall_width = g_screen_size_x;
xpos = 12; //FIXME X POS WFALL

//Draw first line of waterfall
for(point=0;point<1024;point++) //FFT SIZE
    {
    //FIXME something like this needed here ??? 
    //level_db = kiwi_buf[n] - 25 + (3*g_zoom); //should be compensated for each Zoom value
    inx = (int) 130+(kiwi_buf[point]); //adjusted
    colour = get_colour(inx);
    set_pixel(wfall_buf,point + xpos , 0, colour);

    if(INTERP ==1) //Interp from 1024 bins to 1280 pixels. i.e 1 extra pixel per 4 drawn.
        {
        if(xtra > 3)
            {
            xtra = 0;
            xpos++;
            set_pixel(wfall_buf,point + xpos , 0, colour);
            }
        }
    xtra++;
    }

//Scroll all lines down, starting from the bottom
for(int line = WFALL_HEIGHT; line >=0 ; line--)
    {
    for(int x = 0;x<wfall_width;x++)
        {
        wfall_buf[((line+1)*wfall_width)+wfall_width+x] = wfall_buf[(line * wfall_width)+x];
        }
    }
copy_surface_to_framebuf(wfall_buf,xpos,WFALL_POS,g_screen_size_x,WFALL_HEIGHT);
}



//===================================================

int main()
{
// Open the framebuffer device file for reading and writing
fbfd = open("/dev/fb0", O_RDWR); 
if (fbfd == -1) 
    printf("Error: cannot open framebuffer device.\n");
 
if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) // Get variable screen information
	    printf("Error reading variable screen info.\n");
printf("Display info %dx%d, %d bpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );

//https://www.eevblog.com/forum/microcontrollers/(linux)-inhibit-kernel-messages-and-cursor  - hide cursor
char *kbfds = "/dev/tty";
int kbfd = open(kbfds, O_WRONLY);
if (kbfd >= 0) {
    ioctl(kbfd, KDSETMODE, KD_GRAPHICS);
    }
    else {
        printf("Could not open %s.\n", kbfds);
    }

g_screen_size_x = vinfo.xres;
screen_size_y = vinfo.yres;
bytes_pp = vinfo.bits_per_pixel/8;

int fb_data_size = g_screen_size_x * screen_size_y * bytes_pp;
printf (" FB data size = %d \n",fb_data_size);

spec_buf = malloc(g_screen_size_x*SPEC_HEIGHT*bytes_pp);
wfall_buf = malloc(g_screen_size_x*WFALL_HEIGHT*bytes_pp);
cmd_buf = malloc(g_screen_size_x*CMD_HEIGHT*bytes_pp);
fscale_buf = malloc(g_screen_size_x*SCALE_HEIGHT*bytes_pp);

// map framebuffer to user memory 
frame_buf = (uint16_t * ) mmap(0, fb_data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
clear_screen(rgb565(0,3,0));

//make Spectrum frame
plot_thick_rectangle(frame_buf,0,0,g_screen_size_x,SPEC_HEIGHT+10,C_BLUE);
plot_large_string(frame_buf,320,600,"WAITING FOR KIWI",C_WHITE);
//draw_freq_scale();

//Start the KIWI callback thread
//pthread_create(&kiwi_callback,NULL, (void *) setup_kiwi,NULL);

printf(" SETUP ==========================  \n");


draw_spectrum(C_BLACK);

do_network_setup();

printf(" END NW SETUP \n");
/*
 int n = recvfrom(sockfd_1, & cmd_msg_buffer, MAX_IN_BUF,MSG_DONTWAIT, NULL,NULL); 
    if(n > 0) 
        command_selection(n);

  packet_buf_a[0] = 0x42; //Force ID type to be video 1
    sendto(sockfd_1, packet_buf_a, 1024, 0, (struct sockaddr *) &	cliaddr_1, sizeof(cliaddr_1));
*/

//Make command tabs
g_ntabs = CMD_TABS;
g_tab_width = g_screen_size_x / g_ntabs;
g_tab_height = TAB_HEIGHT;

//for(int t = 0; t < g_ntabs;t++)
 //   plot_thick_rectangle(cmd_buf,t * g_tab_width,0,(t*g_tab_width) + g_tab_width,CMD_HEIGHT-6,C_YELLOW);


//Show start-up defaults ( this ain't right though!)
cmd_select = 0;

copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT); 

//Start the Shuttle thread.
//pthread_create(&shuttle_callback,NULL, (void *) shuttle,NULL);

while(1)
    {

    usleep(100000);
    }

}
