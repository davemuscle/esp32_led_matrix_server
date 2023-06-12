#!/bin/python

from PIL import Image
import serial
import time
import sys

ser = serial.Serial('/dev/ttyUSB0', 115200)

im = Image.open(sys.argv[1])
rgb = im.getpixel((0,0))

for y in range(0,64):
    for x in range(0,64):
        rgb = im.getpixel((x,y))
        #rgb = (255&(x<<4), 0, 0)
        print(rgb)
        ser.write(int.to_bytes(rgb[0]))
        ser.write(int.to_bytes(rgb[1]))
        ser.write(int.to_bytes(rgb[2]))
