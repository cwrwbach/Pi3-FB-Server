#pragma once


#define MAX_FREQ 30000
#define FFT_SIZE 1024

#define FRAME_BUF_HEIGHT 600 //768
#define SPEC_HEIGHT 100
#define SPEC_BASE_LINE 159
#define WFALL_HEIGHT 250
#define WFALL_POS 220
#define CMD_TABS 5
#define TAB_HEIGHT 80
#define CMD_HEIGHT 90
#define CMD_POS 470

#define SCALE_HEIGHT 40
#define SCALE_POS 170

#define LEGEND_HEIGHT 200
#define LEGEND_WIDTH 400


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
