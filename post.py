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
            h = open('config_post.txt', 'w')
            print >> h, 'float lightDirX = 1'
            print >> h, 'float lightDirY = -1'
            print >> h, 'float lightDirZ = 1'
            print >> h, 'float r = 0.4'
            print >> h, 'float rpower = 8'
            print >> h, 'float ambient = 0.1'
            print >> h, 'float gi = 0.2'
            print >> h, 'float gin = 0.5'
            print >> h, 'float diff = 0.2'
            print >> h, 'bool groundOcclusion = 1'
            print >> h, 'bool groundShadow = 1'
            h.close()
            stoneID = j
            command = "PostProcess.exe config_post.txt " + str(stoneID) + str(i) + "_diff.tga "+ str(stoneID) + str(i) + "_normal.tga "+ str(stoneID) + str(i) + "_gi_normal.tga build/"+ str(stoneID) + str(i) + "_post.tga"
            print command
            os.system(command)
main()
command = "build/deltga.bat"
os.system(command)