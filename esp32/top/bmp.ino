bool get_bmp(String filename){
    File file = SD.open(filename);
    if(!file){
        return false;
    }

    uint8_t buffer [256];
    uint32_t offset, width, height;
    uint16_t bpp;
    uint32_t bytes_per_row;
    uint16_t rgb;


    file.seek(0x0A);
    file.read((uint8_t *)&offset, 4);
    file.seek(0x12);
    file.read((uint8_t *)&width, 4);
    file.seek(0x16);
    file.read((uint8_t *)&height, 4);
    file.seek(0x1C);
    file.read((uint8_t *)&bpp, 2);

    bytes_per_row = 4 * (bpp * width) / 32;

    uint32_t x = 0, y = 0;
    if(width < DISPLAY_WIDTH){
        x = (DISPLAY_WIDTH - width) >> 1;
    }
    if(height < DISPLAY_HEIGHT){
        y = (DISPLAY_HEIGHT - height) >> 1;
    }

    for(int j = 0; j < height; j++) {
        file.seek(offset + (j*bytes_per_row));
        file.read(buffer, sizeof(buffer));
        int k = 0, i = 0;
        rgb = 0;
        while((k < sizeof(buffer)) && (i < width) && (i < DISPLAY_WIDTH)){
            if(bpp >= 24){
                rgb = ((buffer[k+0] >> 3) <<  0) + // blue
                      ((buffer[k+1] >> 2) <<  5) + // green
                      ((buffer[k+2] >> 3) << 11);  // red
            } else {
                rgb = buffer[k+0] + (buffer[k+1] << 8);
            }
            switch(bpp){
                case(32): k += 4; break;
                case(24): k += 3; break;
                default : k += 2;
            }
            img_buff[DISPLAY_HEIGHT-1-j-y][x+i] = rgb;
            i++;
        }
        if(j == DISPLAY_HEIGHT-1) break;
    }
    file.close();
    return true;
}
