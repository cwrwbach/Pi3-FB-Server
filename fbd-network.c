//#include <vws/websocket.h>
#include <unistd.h>
#include <linux/kd.h>
#include "fbd-colours.h"
#include <stdint.h>
#include <inttypes.h>
#include <linux/kd.h>

typedef unsigned int uint;

#define FFT_SIZE 1024

extern int8_t fbd_buf[FFT_SIZE];
//vws_cnx* cnx;
int watch_dog;

extern uint g_centre_freq;
extern int g_zoom;
extern int g_speed;
extern uint g_url;


void draw_spectrum(short);
void draw_waterfall();

//============================
#if 0
void * setup_kiwi()
{
cnx = vws_cnx_new();    
char uri_string[256];
static double log_fft[1024];
char snd_txt[64];

// Set connection timeout to 5 seconds (the default is 10). This applies
// both to connect() and to read operations (i.e. poll()).
vws_socket_set_timeout((vws_socket*)cnx, 5);
time_t utc_now = time( NULL );
printf(" utc %ld \n" , utc_now);

//http://81.168.1.206:8073/
//http://shack2.ddns.net:8073/
//http://80m.live:8079/
//Complete 'GET' header string is:
//sprintf(uri_string,"ws://81.168.1.206:8073/%d/W/F",utc_now);

sprintf(uri_string,"ws://norsom.proxy.kiwisdr.com:8073/%ld/W/F",utc_now);
printf("Header string: %s\n",uri_string);

if (vws_connect(cnx, uri_string) == false)
    {
    printf("Failed to connect to the WebSocket server\n");
    vws_cnx_free(cnx);
    return(NULL);
    }

// Can check connection state this way. 
assert(vws_socket_is_connected((vws_socket*)cnx) == true);

// Enable tracing - dump frames to the console in human-readable format.
//vws.tracelevel = VT_PROTOCOL;

//Commands to the KIWISDR to set up a waterfall
// Send a TEXT frame
vws_frame_send_text(cnx, "SET auth t=kiwi p=");
usleep(100000);
sprintf(snd_txt,"SET zoom=%d cf=%d",g_zoom,g_centre_freq);
vws_frame_send_text(cnx,snd_txt);
usleep(100000);
vws_frame_send_text(cnx,"SET maxdb=-50 mindb=-110");
usleep(100000);
sprintf(snd_txt,"SET wf_speed=%d",g_speed);
vws_frame_send_text(cnx,snd_txt);
usleep(100000);
vws_frame_send_text(cnx,"SET wf_comp=0");
usleep(100000);
vws_frame_send_text(cnx,"SET ident_user=Squire");

watch_dog=0;    

int8_t temp;
char show[32];
while(1)
    {   
    vws_msg* reply = vws_msg_recv(cnx);

    if (reply == NULL)
        {
        printf(" No Message  recd. Line: %d \n",__LINE__);
        // There was no message received and it resulted in timeout
        }
    else
        {
        // Free message
        //printf(" Received: %d \n",debug++);
        if(watch_dog++ > 30)
            {
            watch_dog = 0;
            vws_frame_send_text(cnx,"SET keepalive");
            }

        if(strncmp("W/F",(char *) reply->data->data,3)==0)
            {
            for(int i = 0; i< 1024;i++)
                {
                temp = reply->data->data[i+16]; 
                if(temp > 0) 
                    temp = -120; //bug fudger FIXME (SPIKE SMOOTHER)
        
                log_fft[i] -= 0.3f * (log_fft[i] - temp );// Smaller factor increase the average time
                //log_fft[i] = temp; //BYPASS AVERAGE
                kiwi_buf[i] = (int8_t) log_fft[i] ; //temp ; //signed dB
                }

            strncpy(show,(char *) reply->data->data,16);
            }
        vws_msg_free(reply);   
 
        draw_spectrum(C_GREEN);
        draw_waterfall();      
        }
    }   
}
#endif
