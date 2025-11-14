#include "fbd-conf.h"
#include "fbd-lib.h"
#include "fbd-colours.h"
#include "fbd-jet.h"
#include <linux/kd.h>
#include <inttypes.h>

#define INTERP 1

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

//void update_zoom();
//void update_speed();
//void update_cf();
//void clear_rectangle();
//void draw_freq_scale();

//================

void draw_grid()
{
int i;
int n_horiz;
int h_gap;

n_horiz=4;
h_gap = SPEC_HEIGHT/(n_horiz);

for(i=1;i<n_horiz;i++)
    plot_dotted_line(spec_buf,0,i*h_gap,g_screen_size_x,i*h_gap,C_YELLOW);//YELLOW);

plot_dotted_line(spec_buf,g_screen_size_x/2,0,g_screen_size_x/2,SPEC_HEIGHT,C_YELLOW);//YELLOW);

plot_large_string(spec_buf,1300,20," -60",C_WHITE);
plot_large_string(spec_buf,1300,60," -80",C_WHITE);
plot_large_string(spec_buf,1300,100,"-100",C_WHITE);
plot_large_string(spec_buf,1300,140,"-120",C_WHITE);
}

void draw_freq_scale()
{
char f_left[32];
char f_centre[32];
char f_right[32];

for(int b=0;b<SCALE_HEIGHT * g_screen_size_x;b++)
    fscale_buf[b] = 0x8000;

sprintf(f_left,"Lefthand ");
sprintf(f_centre,"%d",g_centre_freq);
sprintf(f_right,"Rightthumb ");

plot_large_string(fscale_buf,5,20,f_left,C_WHITE);
plot_large_string(fscale_buf,(g_screen_size_x/2) - 50,20,f_centre,C_WHITE);
plot_large_string(fscale_buf,g_screen_size_x -100,20,f_right,C_WHITE);

plot_large_string(fscale_buf,g_centre_freq-10000,20,">|<",C_GREEN);
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
    spec_buf[b] = 0x0006; //0x0004;

draw_grid();
xpos = 12; //left hand start pos of spectrum

for(int n = 1; n < FFT_SIZE; n++)
    {
    level_db = kiwi_buf[n] - 25 + (3*g_zoom); //should be compensated for each Zoom value
    //120 as minimum - gives us db120
    screen_val= 120 + level_db; //positive relatve to -120
    screen_val *=2; //Scale up * 2
    //Guard against over value
    if(screen_val <0) screen_val =0;
    if(screen_val >= SPEC_HEIGHT) screen_val = SPEC_HEIGHT-1;
    //screen origin is bottom left.
    plot_line(spec_buf,xpos,spec_base , xpos,spec_base - screen_val,colour); //Plots pos've from bottom left.
    
    if(INTERP ==1) //Interp from 1024 bins to 1280 pixels. i.e 1 extra pixel per 4 drawn.
        {
        if(xtra > 3)
            {
            xtra = 0;
            xpos++;
            plot_line(spec_buf,xpos,spec_base , xpos,spec_base - screen_val,colour); //Plots pos've from bottom left.
            }
        }
    xpos++;
    xtra++;
    }
copy_surface_to_framebuf(spec_buf,0,6,g_screen_size_x,SPEC_HEIGHT);
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

//---

void clear_tab(int tab,uint16_t colour)
{
clear_rectangle(cmd_buf,(tab * g_tab_width)+3, 4, (tab * g_tab_width)+g_tab_width-4,g_tab_height,colour);
}


void do_tab_right() // Plus on cmd select
{
int t;
cmd_select++;
if(cmd_select >4) cmd_select = 0;
for(t = 0; t < g_ntabs;t++)
    {
    plot_thick_rectangle(cmd_buf,t * g_tab_width,0,(t*g_tab_width) + g_tab_width,CMD_HEIGHT-6,C_YELLOW);
    if( t == cmd_select)
	plot_thick_rectangle(cmd_buf,t * g_tab_width,0,(t*g_tab_width) + g_tab_width,CMD_HEIGHT-6,C_RED);
    }
//t=cmd_select;
//plot_thick_rectangle(cmd_buf,t * g_tab_width,0,(t*g_tab_width) + g_tab_width,CMD_HEIGHT-6,C_RED);
copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);

//cmd_select++;
//if(cmd_select >4) cmd_select = 0;
}

void do_tab_left() //Neg on cmd select
{
int t;
cmd_select--;
if(cmd_select <0) cmd_select = 4;

for(t = 0; t < g_ntabs;t++)
    {
    plot_thick_rectangle(cmd_buf,t * g_tab_width,0,(t*g_tab_width) + g_tab_width,CMD_HEIGHT-6,C_YELLOW);
    if( t == cmd_select)
	plot_thick_rectangle(cmd_buf,t * g_tab_width,0,(t*g_tab_width) + g_tab_width,CMD_HEIGHT-6,C_RED);
    }

copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);
}

void update_speed(int spd)
{
char cmd_string[32];
char ss[32];

g_speed +=spd;
if(g_speed > 3) g_speed = 3;
if(g_speed <0) g_speed = 0;

//update focus tab
clear_tab(3,C_BLACK);
sprintf(cmd_string,"Speed: %d ",g_speed);
plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);

//update kiwi			
//sprintf(ss,"SET wf_speed=%d",g_speed);
//vws_frame_send_text(cnx,ss);
//usleep(10000);
}

void update_zoom(int zm)
{
char cmd_string[32];
char sz[32];

g_zoom +=zm;
if(g_zoom > 14) g_zoom = 14;
if(g_zoom < 0 ) g_zoom = 0;

//update focus tab
clear_tab(ZOOM,C_BLACK);
sprintf(cmd_string,"Zoom: %d ",g_zoom);
plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);

//update kiwi	
sprintf(sz,"SET zoom=%d cf=%d",g_zoom,g_centre_freq);
//vws_frame_send_text(cnx,sz);
usleep(10000);
}


void update_cf (int cf)
{
char cf_str[32];
char sz[32];

g_centre_freq += cf;
if(g_centre_freq > 30000) g_centre_freq = 30000;
if(g_centre_freq <0) g_centre_freq = 0;


clear_tab(2,C_BLACK);
sprintf(cf_str,"CF: %d ",g_centre_freq);
plot_large_string(cmd_buf,(2 * 275) +50,40,cf_str,C_YELLOW);
copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);

//printf("CF: %d \n",g_centre_freq);
draw_freq_scale();

//update kiwi	
sprintf(sz,"SET zoom=%d cf=%d",g_zoom,g_centre_freq);
//vws_frame_send_text(cnx,sz);
usleep(10000);
}


//======

int main()
{
g_centre_freq = 10000;

fbfd = open("/dev/fb0", O_RDWR); // Open the framebuffer device file for reading and writing
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
draw_freq_scale();

//defaults: need work. FIXME
g_zoom = 0;
g_centre_freq = 15000;
g_speed = 2;
//Start the KIWI callback thread
//pthread_create(&kiwi_callback,NULL, (void *) setup_kiwi,NULL);

printf(" SETUP ==========================  \n");




//Make command tabs
g_ntabs = CMD_TABS;
g_tab_width = g_screen_size_x / g_ntabs;
g_tab_height = TAB_HEIGHT;

//for(int t = 0; t < g_ntabs;t++)
 //   plot_thick_rectangle(cmd_buf,t * g_tab_width,0,(t*g_tab_width) + g_tab_width,CMD_HEIGHT-6,C_YELLOW);


//Show start-up defaults ( this ain't right though!)
cmd_select = 0;
do_tab_left(0);

/*
draw_home_cmd();
draw_speed_cmd();
draw_cf_cmd();
draw_zoom_cmd();
draw_uri_cmd();
* */
copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT); 

//Start the Shuttle thread.
//pthread_create(&shuttle_callback,NULL, (void *) shuttle,NULL);

while(1)
    {

    usleep(100000);
}
#if 0
    if(sx_ring != 0 ) //&& cmd_select == CF) ???
        {
        g_centre_freq += sx_ring *20;
       	update_cf(0);
	}

    if(sx_dial != 0)
        {
	switch (cmd_select)
	    {
	    case ZOOM:
	    update_zoom(sx_dial);
	    break;

	    case CF:
	    g_centre_freq += sx_dial;
	    update_cf(sx_dial);
	    break;

	    case SPEED:
	    update_speed(sx_dial);
	    break;
	    }
	sx_dial = 0;
        }

    if(btn_0 ==1)
        {
        btn_0 = 0;
	do_tab_left();
        }
    if(btn_1 ==1)
        {
        btn_1 = 0;
	do_tab_right();
        }
    if(btn_2 ==1)
        {
        btn_2 = 0;
        printf(" \n TWO \n \n");
        }
    if(btn_3 ==1)
        {
        btn_3 = 0;
	printf(" \n THREE \n \n");
        }
    if(btn_4 ==1)
        {
        btn_4 = 0;
        printf(" \n FOUR \n \n");
        }

    //shuttle();
    //waiting for C2C commands etc.    
    }
#endif
}
