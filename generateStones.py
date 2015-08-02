import os
import sys
import zlib
import fnmatch
import subprocess
import tempfile
import re
import shutil
import platform
import zipfile
import time
import random
import atexit
import colorama
from colorama import Fore, Back, Style
from glob import glob

# --------------------------------------------------------------------------------------------------------------------
def PrintInformationOnBuildIsFinished(startTimeInSeconds):
    timeDelta = time.gmtime(time.time() - startTimeInSeconds)
    print Style.BRIGHT + Fore.GREEN + time.strftime("Build time: %M minutes %S seconds", timeDelta) + Style.RESET_ALL
	
def main():
    colorama.init()
    timeStart = time.time()
    atexit.register(PrintInformationOnBuildIsFinished, timeStart)
    random.seed(2)
    j = 1
    while j<10:
        j+=1
        i = 0
        while i<3:
            i+=1
            h = open('config.txt', 'w')
            print >> h, 'int width = 512'
            print >> h, 'int height = 512'
            print >> h, 'int voxelCount = 5'
            print >> h, 'bool groundOcclusion = 1'
            print >> h, 'bool groundShadow = 0'
            print >> h, 'bool moss = 0'
            print >> h, 'bool dust = 1'
            print >> h, 'str mosstex = moss.tga'
            print >> h, 'str dusttex = dust.tga'
            print >> h, 'float scale = 1.1'
            print >> h, 'float offsetx = 1'
            print >> h, 'float offsety = 1'
            print >> h, 'float lightDirX = -1'
            print >> h, 'float lightDirY = 3'
            print >> h, 'float lightDirZ = -1'
            print >> h, 'float dustSideX  = -1'
            print >> h, 'float dustSideY  = 0'
            print >> h, 'float dustSideZ  = -1'
            print >> h, 'float detailTExMul   = 1'
            print >> h, 'float rotx    = %f' % (random.random()*2.0*3.14)
            print >> h, 'float roty    = %f' % (random.random()*2.0*3.14)
            print >> h, 'float rotz    = %f' % (random.random()*2.0*3.14)
            h.close()
            stoneID = j
            command = "PICORENDER.exe stone" + str(stoneID) + ".obj DSC_4272.tga " + str(stoneID) + str(i)
            print command
            os.system(command)
main()