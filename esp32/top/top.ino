
#include "top.h"


AnimatedGIF gif;

HUB75_I2S_CFG::i2s_pins _pins = {LM_R1, LM_B1, LM_G1, LM_R2, LM_B2, LM_G2, LM_A, LM_B, LM_C, LM_D, LM_E, LM_LAT, LM_OE, LM_CLK};
MatrixPanel_I2S_DMA *ledmatrix = nullptr;

TaskHandle_t task_server_handle;

void setup() {

    // Serial
    Serial.begin(115200);
    while (!Serial);


    // Debugging power-up display issues:
    pinMode(LM_CLK, OUTPUT);
    pinMode(LM_LAT, OUTPUT);
    pinMode(LM_OE, OUTPUT);
    digitalWrite(LM_CLK, 0);
    digitalWrite(LM_LAT, 0);
    digitalWrite(LM_OE,  1);
    delay(100);
    for(int i = 0; i < 64; i++) {
        digitalWrite(LM_CLK, 1);
        digitalWrite(LM_CLK, 0);
    }
    delay(100);

    // LED Matrix
    HUB75_I2S_CFG mxconfig (64, 64, 1, _pins);
    mxconfig.double_buff = true;
    //mxconfig.clkphase = false;
    ledmatrix = new MatrixPanel_I2S_DMA(mxconfig);
    ledmatrix->begin();
    ledmatrix->setBrightness8(20);
    ledmatrix->setTextColor(ledmatrix->color444(15,15,15));
    ledmatrix->setTextSize(1);
    ledmatrix->setTextWrap(true);

    // File System
    if (!SD.begin(5)) {
        String err = "SD card error!";
        ledmatrix->setCursor(0,0);
        ledmatrix->clearScreen();
        ledmatrix->println(err);
        ledmatrix->flipDMABuffer();
        Serial.println(err);
        return;
    }

    // Read the SSID and password
    String ssid, pass;
    File cred_h = SD.open("/credentials.h");
    if (!cred_h) {
        String err = "No credentials!";
        ledmatrix->setCursor(0,0);
        ledmatrix->clearScreen();
        ledmatrix->println(err);
        ledmatrix->flipDMABuffer();
        Serial.println(err);
        return;
    } else {
        Serial.println("Reading credentials");
        ssid = cred_h.readStringUntil('\n');
        pass = cred_h.readStringUntil('\n');
        ssid.trim();
        pass.trim();
        Serial.println("Network SSID: " + ssid);
        Serial.println("Network pass: " + pass);
    }

    // Wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("STA: Failed!\n");
    }
    String ip_line = "IP:\n" + WiFi.localIP().toString();
    Serial.println(ip_line);
    ip_line.replace('.', '.\n');
    ledmatrix->setCursor(0,0);
    ledmatrix->clearScreen();
    ledmatrix->println(ip_line);
    ledmatrix->flipDMABuffer();
    if (WiFi.scanComplete() == -2) WiFi.scanNetworks(true);


    // add delay for IP display
    delay(1000);

    // Index File
    init_index();

    // Webserver
    setup_webserver();

    gif.begin(LITTLE_ENDIAN_PIXELS);

    xTaskCreatePinnedToCore(
        task_server,
        "task_server",
        10000,
        NULL,
        1,
        &task_server_handle,
        0
    );

}

void task_server (void *pvParameters){
    Serial.print("Server task running on core ");
    Serial.println(xPortGetCoreID());
    while(1){
        webserver.handleClient();
        delay(1);
    }
}

bool restart = true;
index_t item;

void loop() {

    int flag;
    String name;
    bool draw = false;
    File file;

    fsmutex.lock();
    while(!delete_queue.empty()){
        String path = "/" + delete_queue.back();
        Serial.println("Attempting to remove: " + path);
        if(SD.exists(path.c_str())){
            SD.remove(path.c_str());
        }
        delete_queue.pop_back();
    }
    flag = traverse_index(&item, restart, true);
    fsmutex.unlock();

    restart = false;
    
    if(flag == -1) {
        restart = true;
        delay(5000);
        return;
    }

    name = "/" + String((const char *)item.name);

    if(name.endsWith(".txt")){
        Serial.println("Displaying text from file: " + name);
        file = SD.open(name);
        if(file) {
            ledmatrix->setCursor(0,0);
            ledmatrix->clearScreen();
            while(file.available()){
                ledmatrix->println(file.readStringUntil('\n'));
            }
            ledmatrix->flipDMABuffer();
            file.close();
        } else {
            Serial.println("Failed to open!");
        }
        draw = true;
    } else if(name.endsWith(".bmp") || name.endsWith(".jpg")){
        Serial.println("Displaying image from file: " + name);
        memset(img_buff, 0, IMG_BUFF_SIZE);
        if((name.endsWith(".bmp") && get_bmp(name)) ||
           (name.endsWith(".jpg") && get_jpg(name))) {
            draw = true;
        } else {
            Serial.println("Failed to open!");
        }
        if(draw){
            for(int j = 0; j < DISPLAY_HEIGHT; j++) {
                for(int i = 0; i < DISPLAY_WIDTH; i++) {
                    ledmatrix->drawPixel(i,j, img_buff[j][i]);
                }
            }
            ledmatrix->flipDMABuffer();
        }
    } else if(name.endsWith(".gif")){
        Serial.println("Displaying gif from file: " + name);
        memset(img_buff, 0, IMG_BUFF_SIZE);
        for(int i = 0; i < IMG_LOOPS; i++){
            if(gif.open(name.c_str(), GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw)){
                gif_vx = 0;
                gif_vy = 0;
                if(gif.getCanvasWidth() < DISPLAY_WIDTH){
                    gif_vx = (DISPLAY_WIDTH - gif.getCanvasWidth()) >> 1;
                }
                if(gif.getCanvasHeight() < DISPLAY_HEIGHT){
                    gif_vy = (DISPLAY_HEIGHT - gif.getCanvasHeight()) >> 1;
                }
                int fd = 0;
                while(gif.playFrame(true,&fd)){
                    for(int j = 0; j < DISPLAY_HEIGHT; j++) {
                        for(int i = 0; i < DISPLAY_WIDTH; i++) {
                            ledmatrix->drawPixel(i,j, img_buff[j][i]);
                        }
                    }
                    ledmatrix->flipDMABuffer();
                }
                gif.close();
            } else {
                Serial.println("Failed to open!");
            }
        }
    }

    if(draw) delay(IMG_DELAY);

}


// TODO:
/*
Implement move up/down feature for webserver
Use chunked response for html page
*/


