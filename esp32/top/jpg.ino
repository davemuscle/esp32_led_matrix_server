
JPEGDEC jpeg;
File jpeg_file;

bool get_jpg (String filename) {
    if(!jpeg.open(filename.c_str(), cb_jpeg_open, cb_jpeg_close, cb_jpeg_read, cb_jpeg_seek, cb_jpeg_draw)) {
        return false;
    }
    uint32_t vx = 0, vy = 0;
    if(jpeg.getWidth() < DISPLAY_WIDTH){
        vx = (DISPLAY_WIDTH - jpeg.getWidth()) >> 1;
    }
    if(jpeg.getHeight() < DISPLAY_HEIGHT){
        vy = (DISPLAY_HEIGHT - jpeg.getHeight()) >> 1;
    }
    jpeg.decode(vx,vy,0);
    jpeg.close();
    return true;
}

int cb_jpeg_draw (JPEGDRAW *pDraw){
    int w, h, x, y;
    w = pDraw->iWidth;
    h = pDraw->iHeight;
    x = pDraw->x;
    y = pDraw->y;
    for(int j = 0; j < h; j++) {
        for(int i = 0; i < w; i++) {
            uint16_t rgb;
            rgb = pDraw->pPixels[i + j*w];
            img_buff[y+j][x+i] = rgb; // extern
        }
    }
    if(((x+w) > DISPLAY_WIDTH) || ((y+h) > DISPLAY_HEIGHT))
        return 0;
    return 1;
}

void *cb_jpeg_open (const char *filename, int32_t *size){
    jpeg_file = SD.open(filename);
    *size = jpeg_file.size();
    return &jpeg_file; 
}

void cb_jpeg_close (void *handle){
    if(jpeg_file) jpeg_file.close();
} 

int32_t cb_jpeg_read (JPEGFILE *handle, uint8_t *buffer, int32_t length){
    if(!jpeg_file) return 0;
    return jpeg_file.read(buffer,length);
}

int32_t cb_jpeg_seek (JPEGFILE *handle, int32_t position){
    if(!jpeg_file) return 0;
    return jpeg_file.seek(position);
}

