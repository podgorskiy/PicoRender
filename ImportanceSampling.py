import sys
from PIL import Image
from math import * 
import random

def writePoint(image, pixels, x, y, p):
    w = image.size[0]
    h = image.size[1]
    i = (1.0 - y)/2.0 * h
    j = (1.0 + x)/2.0 * w
    (r,g,b,a) = p
    pixels[i,j] = (int(r * 255), int(g * 255), int(b * 255), int(a * 255))


def GenerateLUT(filename, sizeW, sizeH, type):
    image = Image.new(type, (sizeW,sizeH))
    pixels = image.load() # create the pixel map

    for i in range(image.size[0]):    # for every pixel:
        x = float(i) / image.size[0]
        for j in range(image.size[1]):
            y = 1.0 - float(j) / image.size[1]
            (r,g,b,a) = (0,0,0,1)
            pixels[i,j] = (int(r * 255), int(g * 255), int(b * 255), int(a * 255))
    
    number = 256
    weight = 1.0 / number
    I = 0.0
    for i in range(1, number):
        ro = pow(random.random(), 0.5)
        th = random.random() * 2.0 * pi
        x = ro * cos(th)
        y = ro * sin(th)
        writePoint(image, pixels, x, y, (1,1,1,1))
        I += (1-ro) * weight 
    image.save("n_"+filename, "TGA")
    I *= pi
    print(I)

    circles = 16
    rays = 16
    weight = 1.0 / (circles * rays)
    I = 0.0

    for i in range(1, circles+1):
        for j in range(1, rays+1):
            ro = pow(random.uniform((i-1.0) / float(circles), i / float(circles)), 0.5)
            th = random.uniform((j-1.0) / float(rays), j / float(rays)) * 2.0 * pi
            x = ro * cos(th)
            y = ro * sin(th)
            writePoint(image, pixels, x, y, (1,1,1,1))
            I += (1-ro) * weight 
    image.save("s_"+filename, "TGA")
    I *= pi
    print(I)


GenerateLUT("MonteCarlo.tga", 512, 512, "RGBA") 