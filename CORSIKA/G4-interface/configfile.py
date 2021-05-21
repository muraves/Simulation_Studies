#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Jan 12 12:27:25 2021

@author: amrutha
"""

import os

#enter the file name you want to create (filename.i)
text_file = open("corsika2asci.i", "w")
#enter total no of showers
tot_noof_shwr= 600
 #enter the no of showers per file
noof_shwr_prfile=10
#enter the path of the folder of CORSIKA binary files inside the brackets
path, dirs, files = next(os.walk("/home/amrutha/CORSIKA/dpmjet/src/utils/binary_files_flujo_central_europe-60sec"))


file_count = len(files) #gives the no of files in the folder


text_file.write('%d' % tot_noof_shwr)
text_file.writelines('\n')
text_file.write('%d' %file_count)
text_file.writelines('\n')


for root, dirs, files in os.walk(os.path.abspath(path)):
    for file in files:
        file_path=os.path.join(root, file)
        text_file.writelines('"'+file_path+'"')
        text_file.writelines('\n')
        text_file.write('%d' % noof_shwr_prfile)
        text_file.writelines('\n')
text_file.close()

new_dir = "g4_ascii"
if not os.path.exists(new_dir):
    os.makedirs(new_dir)
    
#dir_path=pathlib.Path("configfile.py").parent.absolute()
dir_path=os.path.dirname(os.path.abspath("configfile.py"))

   
os.chdir(dir_path+"/g4_ascii")   
os.system("readcsk2asci < %s/corsika2asci.i"%dir_path) 


