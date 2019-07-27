#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h> 
#include <curl/curl.h>
#include <imgproc.h>

#define NUM_REGIONS                2 //specify number of regions used for detection (at least 2 for accuracy reasons)
#define IMAGE_WIDTH              352 //specify horizontal resolution
#define IMAGE_HEIGHT             288 //specify vertical resolution
#define DOMOTICZ_IP    "192.168.1.2" //specify domoticz server IP
#define DOMOTICZ_PORT         "8080" //specify domoticz server PORT
#define DOMOTICZ_IDX            "60" //specify domoticz device IDX


typedef struct _REGION {
   unsigned int x, y;
   unsigned int w, h;
} REGION;

//REGION coordinates definition
REGION region[] =
{
    {270, 200, 15, 15},
    {250, 255, 15, 15}
};

Viewer *view = NULL;
Camera *cam  = NULL;

static int regionHit(Image *img) {

   unsigned int i;
   unsigned int x, y;
   unsigned int rx, ry, rw, rh;
   unsigned int count_dark;
   unsigned char red;
   unsigned char green;
   unsigned char blue;
   unsigned char *pixel;

   for (i = 0; i < NUM_REGIONS; i++) {
      count_dark = 0;
      rx = region[i].x;
      ry = region[i].y;
      rw = region[i].w;
      rh = region[i].h;

      // Count number of dark pixels in given region
      for (x = rx; x < rx + rw; x++) {
         for (y = ry; y < ry + rh; y++) {
            // Get a pointer to the current pixel
            pixel = (unsigned char *)imgGetPixel(img, x, y);

            // index 0 is blue, 1 is green and 2 is red
            red = pixel[2];
            green = pixel[1];
            blue = pixel[0]; 

            // check if pixel is dark
            if (red > 50 && green < 100 && blue < 100){
               count_dark++;
            }
         }
      }

      // We have a hit if more than 80% of the pixels is dark 
      if (count_dark > (rw * rh) * 0.8){
         return i;
      }
   }

   return -1;
}

static void drawRegion(Image *img, REGION region, unsigned char red, unsigned char green, unsigned char blue) {

   unsigned int x, y;
   unsigned int rx, ry, rw, rh;

   rx = region.x;
   ry = region.y;
   rw = region.w;
   rh = region.h;

   y = ry;
   for (x = rx; x < rx + rw; x++) imgSetPixel(img, x, y, blue, green, red);
   y = ry + rh;
   for (x = rx; x < rx + rw; x++) imgSetPixel(img, x, y, blue, green, red);
   x = rx;
   for (y = ry; y < ry + rh; y++) imgSetPixel(img, x, y, blue, green, red);
   x = rx + rw;
   for (y = ry; y < ry + rh; y++) imgSetPixel(img, x, y, blue, green, red);
}

void updateValuesDomoticz(int new_region_number){

	static int last_region_number = -1;
	CURL *curl;
	CURLcode res;
   //FILE *f = fopen("/home/pi/logs/curl_wz_log.txt", "wb");

	if (new_region_number != -1 && last_region_number != new_region_number) {
		last_region_number = new_region_number;
		curl = curl_easy_init();
		if(curl) {
   		curl_easy_setopt(curl, CURLOPT_URL, "http://"DOMOTICZ_IP":"DOMOTICZ_PORT"/json.htm?type=command&param=udevice&idx="DOMOTICZ_IDX"&nvalue=0&svalue=1");
         //curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
   		/* Perform the request, res will get the return code */ 
   		res = curl_easy_perform(curl);
   		/* Check for errors */ 
   		if(res != CURLE_OK)
   			fprintf(stderr, "curl_easy_perform() failed: %s\n",
   			curl_easy_strerror(res));

   			/* always cleanup */ 
   			curl_easy_cleanup(curl);
		}
	}
}

static void cleanup(int sig, siginfo_t *siginfo, void *context) {

   if (view) viewClose(view);
   if (cam) camClose(cam);

   // unintialise the library
   quit_imgproc();

   exit(0);
}

int main(int argc, char * argv[])
{
   int    i;
   int    new_region_number;
   bool   display_image = true;

   // initialise the image library
   init_imgproc();

   // open the webcam 
   cam = camOpen(IMAGE_WIDTH, IMAGE_HEIGHT);
   if (!cam) {
      fprintf(stderr, "Unable to open camera\n");
      fflush(stderr);
      exit(1);
   }

   // create a new viewer of the same resolution with a caption
   if (display_image) {
      view = viewOpen(IMAGE_WIDTH, IMAGE_HEIGHT, "Cold water meter");
      if (!view) {
         fprintf(stderr, "Unable to open view\n");
         fflush(stderr);
         exit(1);
      }
   }

   sleep(1);
   // capture images from the webcam	
   while(1){
      Image *img = camGrabImage(cam);
      if (!img) {
         fprintf(stderr, "Unable to grab image\n");
         fflush(stderr);
         exit(1);
      }

      // check if any region has a hit
      new_region_number = regionHit(img);

      // update accumulated values
      updateValuesDomoticz(new_region_number);

      if (display_image) {
         for (i = 0; i < NUM_REGIONS; i++) {
          
            unsigned char red = 0;
            unsigned char green= 255;
            unsigned char blue = 0;

            if (i == new_region_number) {
               red = 255;
               green = 0;
               blue = 0;
            }
            drawRegion(img, region[i], red, green, blue);
         }
         // display the image to view the changes
         viewDisplayImage(view, img);
      }

      // destroy image
      imgDestroy(img);
   }

   // cleanup and exit
   cleanup(0, NULL, NULL);
   return 0;
}
