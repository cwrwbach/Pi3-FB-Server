#include "fbd-lib.h"
#include "fbd-font.h"
#include "fbd-colours.h"

extern int g_screen_size_x;
extern int screen_size_y;
extern uint16_t * frame_buf;

//---

uint16_t get_colour( int inx)
{
uint16_t colour;

if(inx > 80) 
    {
    colour = rgb565(250, 0,250);
    return colour;
    }

if(inx > 70) 
    {
    colour = rgb565(0x250, 64,0);
    return colour;
    }

if(inx > 60) 
    {
    colour = rgb565(250,0,0);
    return colour;
    }

if(inx > 50) 
    {
    colour = rgb565(0, 64,64);
    return colour;
    }

if(inx > 40) 
    {
    colour = rgb565(32, 32,0);
    return colour;
    }

if(inx > 30) 
    {
    colour = rgb565(0, 32,0);
    return colour;
    }

if(inx > 20) 
    {
    colour = rgb565(0, 8,0);
    return colour;
    }

if(inx > 10) 
    {
    colour = rgb565(0x32, 32,32);
    return colour;
    }

colour = rgb565(0, 0,0);
return colour;
}

void set_pixel(uint16_t * buf,int x, int y, uint16_t colour)
{
int sz_x,sz_y;
int32_t location;

sz_x = g_screen_size_x;
sz_y = screen_size_y;

if(x<0) x=1;
if(x > sz_x-1) x=sz_x-1;
if(y<0) y=0;
if(y>sz_y-1) y=sz_y-1;

location = x + (y*sz_x) ; 

buf[location] = colour;
}

void clear_screen(uint16_t pixval)
{
for(int p=0;p<(g_screen_size_x*(screen_size_y-20));p++)
    frame_buf[p] = pixval;
}

short rgb565(short red,short green,short blue)
{
short colour= 0;
colour = (red & 0xf8) <<8;
colour = colour | ((green & 0xfc) <<3);
colour = colour | (blue >>3);
return colour;
}

void plot_line (uint16_t * buf,int x0, int y0, int x1, int y1,uint16_t colour)
{
int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
int err = dx + dy, e2; //error value e_xy 

for (;;) //evah
    { 
    set_pixel (buf,x0,y0,colour);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0 
    if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0 
    }
}

void plot_dotted_line (uint16_t * buf, int x0, int y0, int x1, int y1,uint16_t colour)
{
int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
int err = dx + dy, e2; //error value e_xy 
int count;
count = 0;
for (;;) //evah
    { 
    if(count++ & 0x04)// miss out spaces
        set_pixel (buf,x0,y0,colour);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0 
    if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0 
    }
}

void plot_thick_line (uint16_t * buf, int x0, int y0, int x1, int y1,uint16_t colour)
{
int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
int err = dx + dy, e2; //error value e_xy 
for (;;) //evah
    { 
    set_pixel (buf,x0,y0,colour);
    set_pixel (buf,x0+1,y0,colour);
    set_pixel (buf,x0,y0+1,colour);
    set_pixel (buf,x0+1,y0+1,colour);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0 
    if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0 
    }
}

void plot_rectangle(uint16_t * buf, int x0, int y0,int sz_x, int sz_y, uint16_t colour)
{
plot_line(buf, x0, y0, x0+sz_x, y0,colour); //top horiz
plot_line(buf, x0, y0+sz_y, x0+sz_x, y0+sz_y,colour); //bot horiz
plot_line(buf, x0, y0, x0, y0+sz_y,colour);
plot_line(buf, x0+sz_x, y0, x0+sz_x, y0+sz_y,colour);
}

void plot_thick_rectangle(uint16_t * buf, int x0, int y0,int sz_x, int sz_y,uint16_t colour)
{
plot_thick_line(buf, x0, y0, x0+sz_x, y0,colour);
plot_thick_line(buf, x0, y0+sz_y, x0+sz_x, y0+sz_y,colour);
plot_thick_line(buf, x0, y0, x0, y0+sz_y,colour);
plot_thick_line(buf, x0+sz_x, y0, x0+sz_x, y0+sz_y,colour);
}

void xxxplot_filled_rectangle(uint16_t * buf, int x0, int y0,int sz_x, int sz_y, uint16_t colour)
{
for(int n = 0; n< sz_y;n++)
    {
    plot_line(buf, x0, y0+n, x0+sz_x, y0+n,colour);
    plot_line(buf, x0, y0+sz_y, x0+sz_x, y0+sz_y,colour);
    plot_line(buf, x0, y0, x0, y0+sz_y,colour);
    plot_line(buf, x0+sz_x, y0, x0+sz_x, y0+sz_y,colour);
    }
}

void clear_rectangle(uint16_t * buf, int x, int y,int xx, int yy, uint16_t colour)
{
for(int line = 1; line < yy;line ++)
    {
    plot_line(buf, x,line,xx,line,colour);
    }
}

void plot_large_character(uint16_t * buf,int x, int y,uint8_t char_num,uint16_t colour)
{
int horiz,vert;
int xx,yy;
unsigned short test_word;

for(vert=0,yy=0; vert<24;vert++,yy+=1)
    {
    test_word = ASCII_16x24[((char_num-32) * 24)+vert]; 
    for(horiz =0,xx=0; horiz<16;horiz++,xx++)
        {
        if(test_word & 0x0001)
            { 
            set_pixel(buf,x+xx,y+yy,colour);
            }
        test_word >>=1; 
        }
    }
}

void plot_large_string(uint16_t * buf , int x, int y,char * string ,uint16_t colour)
{
int len = strlen(string);
char char_num;
int xp = 0;

for(int s=0;s<len;s++,xp++)
    {
    char_num = string[s];
    plot_large_character(buf,x+xp*15,y,char_num,colour);
    }
}


void plot_circle (uint16_t * dest, int xm, int ym, int r,uint16_t colour)
{
int sz_x,sz_y;
sz_x = 50; //dest->sz_x;
sz_y = 50 ; //dest->sz_y;

int x = -r, y = 0, err = 2-2*r; // II. Quadrant 
   do {
      set_pixel (dest,xm-x, ym+y,colour); //   I. Quadrant 
      set_pixel (dest,xm-y, ym-x,colour); //  II. Quadrant 
      set_pixel (dest,xm+x, ym-y,colour); // III. Quadrant 
      set_pixel (dest,xm+y, ym+x,colour); //  IV. Quadrant 
      r = err;
      if (r >  x) err += ++x*2+1; // e_xy+e_x > 0 
      if (r <= y) err += ++y*2+1; // e_xy+e_y < 0 
   } while (x < 0);
}




void copy_surface_to_framebuf(uint16_t *buf,uint loc_x,uint loc_y,uint sz_x,uint sz_y)
{
uint start,size;

start = loc_y * g_screen_size_x;
size = sz_y * g_screen_size_x * 2;
memcpy(frame_buf+start, buf,size);
}

void copy_block_to_fb(uint16_t *buf,uint loc_y,uint sz_y)
{
uint start,size;

start = loc_y * g_screen_size_x;
size = sz_y * g_screen_size_x * 2;
memcpy(frame_buf+start, buf,size);
}

