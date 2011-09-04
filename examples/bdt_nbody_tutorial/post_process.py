#!/usr/bin/pvpython

from paraview.simple import *
import glob, subprocess
from os.path import basename, exists

csv_files=glob.glob('./sample_*.csv')
csv_files.sort()
print csv_files
image_files=[]

for csv_file in csv_files:
    image_file=csv_file+'.png'
    image_files.append(image_file)
    if(not exists(image_file) ):
        reader=OpenDataFile(csv_file)

        filter=TableToPoints(reader)
        filter.XColumn='x'
        filter.YColumn='y'
        filter.ZColumn='z'

        filter.UpdatePipeline()

        geometry=Show(filter)
        geometry.ColorArrayName='m'

        Render()
        WriteImage(image_file)
        Delete()

print image_files
images=','.join(str(image) for image in image_files)
images='mf://'+images
command = ('mencoder',
           images,
           '-mf',
           'type=png:w=400:h=400:fps=4',
           '-ovc',
           'lavc',
           '-lavcopts',
           'vcodec=mpeg4',
           '-oac',
           'copy',
           '-o',
           'output.avi')

print "\n\nabout to execute:\n%s\n\n" % ' '.join(command)
subprocess.check_call(command)
