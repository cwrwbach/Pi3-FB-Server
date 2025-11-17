#pragma once


//#define MAX_FREQ 30000
#define FFT_SIZE 1024

#define FRAME_BUF_HEIGHT 600 //768

#define CHAN_HEIGHT_A 125
#define CHAN_HEIGHT_B 125
#define CHAN_HEIGHT_C 125
#define CHAN_HEIGHT_D 125

#define CHAN_SPACE 130

#define CHAN_POS_A 0
#define CHAN_POS_B CHAN_POS_A+CHAN_SPACE+5
#define CHAN_POS_C CHAN_POS_B+CHAN_SPACE+5
#define CHAN_POS_D CHAN_POS_C+CHAN_SPACE+5

//#define CHAN_POS_C 280
//#define CHAN_POS_D 420

//#define SPEC_BASE_LINE 159





#define WFALL_HEIGHT 250
#define WFALL_POS 220
//#define CMD_TABS 5
//#define TAB_HEIGHT 80
//#define CMD_HEIGHT 90
//#define CMD_POS 470

//#define SCALE_HEIGHT 40
//#define SCALE_POS 170

//#define LEGEND_HEIGHT 200
//#define LEGEND_WIDTH 400


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


//https://www.eevblog.com/forum/microcontrollers/(linux)-inhibit-kernel-messages-and-cursor  - hide cursor
/*
char *kbfds = "/dev/tty";
int kbfd = open(kbfds, O_WRONLY);
if (kbfd >= 0) {
    ioctl(kbfd, KDSETMODE, KD_GRAPHICS);
    }
    else {
        printf("Could not open %s.\n", kbfds);
    }
*/

/*
void draw_spectrum(short colour)
{
int screen_val;
int16_t level_db;
int spec_base;
int xpos;

spec_base = CHAN_POS_A;

draw_grid();

//make Spectrum frame
//plot_thick_rectangle(frame_buf,4,0,g_screen_size_x,SPEC_HEIGHT,C_BLUE);

copy_surface_to_framebuf(chan_buf_a,0,0,g_screen_size_x,CHAN_HEIGHT_A);

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
    if(screen_val >= CHAN_HEIGHT_A) screen_val = CHAN_HEIGHT_A-1;
    //screen origin is bottom left.
  //  plot_line(chan_buf_a,xpos,spec_base , xpos,spec_base - screen_val,colour); //Plots pos've from bottom left.
    plot_line(chan_buf_a,0,100 , 400,0,colour);


    xpos++;
 //   xtra++;
    }

//make Spectrum frame
//plot_thick_rectangle(frame_buf,4,0,g_screen_size_x-10,SPEC_HEIGHT+10,C_BLUE);


colour = C_MAGENTA;
 plot_line(chan_buf_a,0,100 , 900,80,colour);

copy_surface_to_framebuf(chan_buf_a,0,6,g_screen_size_x,CHAN_HEIGHT_A);
printf(" FOUND %d \n",__LINE__);

usleep(500000);
}
*/

//-----


/*
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
    //inx = (int) 130+(kiwi_buf[point]); //adjusted
    //colour = get_colour(inx);
    //set_pixel(wfall_buf,point + xpos , 0, colour);

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
*/
