#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <JPEGDEC.h>

#include <AnimatedGIF.h>

#include "html_static.h"

#define IMG_LOOPS 5
#define IMG_DELAY 5000

#define INDEX_FILE "/index.bin"
#define INDEX_ENTRIES 8

using namespace std;

#include <mutex>
std::mutex fsmutex;

WebServer webserver(80);

#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH 64

#define IMG_BUFF_SIZE DISPLAY_WIDTH*DISPLAY_HEIGHT*2
static uint16_t img_buff [DISPLAY_WIDTH][DISPLAY_HEIGHT];

#define LM_R1  32
#define LM_R2  25
#define LM_G1  22
#define LM_G2  21
#define LM_B1  33
#define LM_B2  26
#define LM_A   27
#define LM_B   16
#define LM_C   14 
#define LM_D   4
#define LM_E   17
#define LM_CLK 12
#define LM_LAT 2
#define LM_OE  13

typedef struct index_t {
    uint32_t idx       = 0;
    bool     base      = false;
    bool     valid     = false;
    uint8_t  name [32] = {0};
    uint16_t padding   = 0;
    uint32_t size      = 0;
    uint32_t next      = 0;
    uint32_t prev      = 0;
};

static std::vector<String> delete_queue;


int gif_vx, gif_vy;

