File gif_file;

void GIFDraw(GIFDRAW *pDraw)
{
    uint8_t *s;
    uint16_t *d, *usPalette;
    int x, iWidth;

    iWidth = pDraw->iWidth;
    if (iWidth + pDraw->iX > DISPLAY_WIDTH)
        iWidth = DISPLAY_WIDTH - pDraw->iX;
    usPalette = pDraw->pPalette;
    if (pDraw->iY + pDraw->y >= DISPLAY_HEIGHT || pDraw->iX >= DISPLAY_WIDTH || iWidth < 1)
        return;
    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) // restore to background color
    {
        for (x=0; x<iWidth; x++)
        {
            if (s[x] == pDraw->ucTransparent)
                s[x] = pDraw->ucBackground;
        }
        pDraw->ucHasTransparency = 0;
    }
    if (pDraw->ucHasTransparency) // if transparency used
    {
        uint8_t c, ucTransparent = pDraw->ucTransparent;
        int x;
        for (x=0; x < iWidth; x++)
        {
            c = *s++;
            if (c != ucTransparent) {
                img_buff[pDraw->iY + pDraw->y + gif_vy][pDraw->iX + x + gif_vx] = usPalette[c];
            }
        }
    }
    else
    {
        // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
        for (x=0; x<iWidth; x++) {
            img_buff[pDraw->iY + pDraw->y + gif_vy][pDraw->iX + x + gif_vx] = usPalette[s[x]];
        }
    }
} /* GIFDraw() */

void * GIFOpenFile(const char *fname, int32_t *pSize)
{
    //Serial.println("Got open");
    gif_file = SD.open(fname);
    if (gif_file)
    {
        *pSize = gif_file.size();
        return (void *)&gif_file;
    }
    return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle)
{
    //Serial.println("Got close");
    File *f = static_cast<File *>(pHandle);
    if (f != NULL)
        f->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    //Serial.println("Got read for " + String(iLen));
    int32_t iBytesRead;
    iBytesRead = iLen;
    File *f = static_cast<File *>(pFile->fHandle);
    // Note: If you read a file all the way to the last byte, seek() stops working
    if ((pFile->iSize - pFile->iPos) < iLen)
        iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
    if (iBytesRead <= 0)
        return 0;
    iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{ 
    //Serial.println("Got seek for " + String(iPosition));
    File *f = static_cast<File *>(pFile->fHandle);
    f->seek(iPosition);
    pFile->iPos = (int32_t)f->position();
    return pFile->iPos;
} /* GIFSeekFile() */
