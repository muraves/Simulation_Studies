#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Jan 26 10:43:14 2021

@author: amrutha
"""

import uproot
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import (MultipleLocator, FormatStrFormatter, AutoMinorLocator)
from matplotlib import colors
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm
import scipy.stats as stats
from scipy.stats import norm
import matplotlib.mlab as mlab
import pickle
import math
from scipy.stats import norm
import random
import collections
import more_itertools as mit
from pandas.core.common import flatten


filee="/home/amrutha/muraves/simulation/MuravesSim_geo_updated/build/z_layer_5p4/MuravesSim.root"
file = uproot.open(filee)
lis=file.keys()
tree=file[lis[0]]
lis1=tree.keys()
eventid=tree.arrays()
data=pd.DataFrame(eventid)
muon_data=data.loc[data.iloc[:, 1]==1]
muon_data=muon_data.loc[muon_data.iloc[:, 7]!=0]
for i in range (0,len(muon_data.iloc[:,3])):
    muon_data.iloc[i,3]=muon_data.iloc[i,3][0]
    muon_data.iloc[i,4]=muon_data.iloc[i,4][0]
    
    print(i)
muon_data.iloc[:,4]=muon_data.iloc[:,4]/1000    
muon_data.iloc[:,10]=muon_data.iloc[:,10]/10
muon_data.iloc[:,11]=muon_data.iloc[:,11]/10
muon_data.iloc[:,12]=muon_data.iloc[:,12]/10

muon_data=muon_data.loc[(muon_data[b'GenPartPDG'] ==-13) | (muon_data[b'GenPartPDG'] ==13) ]

#xdata=np.concatenate( muon_data.iloc[:, 12].to_list(), axis=0 )
#ydata=np.concatenate( muon_data.iloc[:, 12].to_list(), axis=0 )
zdata=np.concatenate( muon_data.iloc[:, 12].to_list(), axis=0 )

station_data=pd.DataFrame()
station_data["event_id"] = ""
station_data["Tot_no_hits"] = ""
for i in range (0,12):
    if (i<4) | (i>7):
        station_data["station_{0}X".format(i)] = ""
        station_data["station_{0}Y".format(i)] = ""
        station_data["station_{0}Z".format(i)] = ""
        station_data["station_{0}HitE".format(i)] = ""
        station_data["station_{0}HitBar".format(i)] = ""
        station_data["station_{0}HitModule".format(i)] = ""

        
        
for a in range (0,len(muon_data)):                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
    station_data.loc[a,"event_id"]=a
    station_data.loc[a,"Tot_no_hits"]=muon_data.iloc[a,7] 
    station_no=muon_data.iloc[a, 13]
    uni_statnno=np.unique(station_no)
    for b in uni_statnno:
        item_index = np.where(station_no==b)
        station_data.loc[a,"station_{0}X".format(b)]=muon_data.iloc[a, 10] [item_index]
        station_data.loc[a,"station_{0}Y".format(b)]=muon_data.iloc[a, 11] [item_index]
        station_data.loc[a,"station_{0}Z".format(b)]=muon_data.iloc[a, 12] [item_index]
        station_data.loc[a,"station_{0}HitE".format(b)] = muon_data.iloc[a, 9] [item_index]
        station_data.loc[a,"station_{0}HitBar".format(b)] = muon_data.iloc[a,15] [item_index]
        station_data.loc[a,"station_{0}HitModule".format(b)] = muon_data.iloc[a,14] [item_index]
    print(a)
    
    
for i in range (0,12):
    if (i<4) | (i>7):
        station_data["station_{0}totE".format(i)] = ""  
        

#
#station_data = station_data.replace(float('nan'), [])
#station_data .fillna([0], inplace=True)
#
#station_data = station_data.replace(0, [0])

station_data=station_data[:].fillna("").apply(list)


        
for a in range (0,len(muon_data)):  
    station_data.iloc[a,50]=sum(station_data.iloc[a,5])           
    station_data.iloc[a,51]=sum(station_data.iloc[a,11])
    station_data.iloc[a,52]=sum(station_data.iloc[a,17])
    station_data.iloc[a,53]=sum(station_data.iloc[a,23])
    station_data.iloc[a,54]=sum(station_data.iloc[a,29])
    station_data.iloc[a,55]=sum(station_data.iloc[a,35])
    station_data.iloc[a,56]=sum(station_data.iloc[a,41])
    station_data.iloc[a,57]=sum(station_data.iloc[a,47])
    
    print(a)
       
xdata=np.concatenate(station_data["station_0X"].to_list(), axis=0 )
ydata=np.concatenate(station_data["station_0Y"].to_list(), axis=0 )

cluster_data=pd.DataFrame()
cluster_data["event_id"] = ""
for i in range (0,12):
    if (i<4) | (i>7):
        cluster_data["HitposX_{0}".format(i)] = ""
        cluster_data["HitposY_{0}".format(i)] = ""
        cluster_data["HitE_per_bar_{0}".format(i)] = ""
        cluster_data["HitBar_{0}".format(i)] = ""
        cluster_data["ClusterposX_{0}".format(i)] = ""
        cluster_data["ClusterposY_{0}".format(i)] = ""
        cluster_data["Cluster_Eng_{0}".format(i)] = ""
        cluster_data["N_Clusters_{0}".format(i)] = ""
        cluster_data["Cluster_Size_{0}".format(i)] = ""


bar_col=[6,12,18,24,30,36,42,48]
col_no=[0,1,2,3,8,9,10,11]
#calculating clusters  

#for a in range (0,len(station_data)): 
#    print(a)
#    for i,j in zip (bar_col,col_no):
#        bar_no=station_data.iloc[a, i]
#        uni_barno=np.unique(bar_no)
#        cluster_data.loc[a,"HitBar_{0}".format(j)]=uni_barno
#        x_arr=[]
#        y_arr=[]
#        e_arr=[]
#        for b in uni_barno:
#             bar_index = np.where(bar_no==b)
#             x_arr.append(np.mean(station_data.iloc[a,i-4] [bar_index]))
#             y_arr.append(np.mean(station_data.iloc[a,i-3] [bar_index]))
#             e_arr.append(np.sum(station_data.iloc[a,i-1] [bar_index]))
#             cluster_data.loc[a,"HitposX_{0}".format(j)]=x_arr
#             cluster_data.loc[a,"HitposY_{0}".format(j)]=y_arr
#             cluster_data.loc[a,"HitE_per_bar_{0}".format(j)]=e_arr
#        bar_list=[list(group) for group in mit.consecutive_groups(uni_barno)]
        
for a in range (0,2000): 
    cluster_data.loc[a,"event_id"]=a
    print(a)
    for i,j in zip (bar_col,col_no):
        bar_no=station_data.iloc[a, i]
        uni_barno=np.unique(bar_no)
        cluster_data.loc[a,"HitBar_{0}".format(j)]=uni_barno
        x_arr=[]
        y_arr=[]
        e_arr=[]
        for b in uni_barno:
             bar_index = np.where(bar_no==b)
#             print(bar_index)
             x_arr.append(np.mean(station_data.iloc[a,i-4] [bar_index]))
             y_arr.append(np.mean(station_data.iloc[a,i-3] [bar_index]))
             e_arr.append(np.sum(station_data.iloc[a,i-1] [bar_index]))
        x_arr=np.array(x_arr)
        y_arr=np.array(y_arr)
        e_arr=np.array(e_arr)
        cluster_data.loc[a,"HitposX_{0}".format(j)]=x_arr
        cluster_data.loc[a,"HitposY_{0}".format(j)]=y_arr
        cluster_data.loc[a,"HitE_per_bar_{0}".format(j)]=e_arr
        bar_list=[list(group) for group in mit.consecutive_groups(uni_barno)] 
        cluster_data.loc[a,"N_Clusters_{0}".format(j)] = len(bar_list)
        cluster_data.loc[a,"Cluster_Size_{0}".format(j)] = len(uni_barno)
        clr_posx=[]
        clr_posy=[]
        clr_eng=[]
        for c in bar_list:
            if len(c)==1:
                clr_posx.append(x_arr [np.where(uni_barno==c[0])])
                clr_posy.append(y_arr [np.where(uni_barno==c[0])])
                clr_eng.append(e_arr [np.where(uni_barno==c[0])])
            else:
                x_cl_arr=[]
                y_cl_arr=[]
                e_cl_arr=[]
                for d in range (len(c)):
                    x_cl_arr.append(x_arr [np.where(uni_barno==c[d])])
                    y_cl_arr.append(y_arr [np.where(uni_barno==c[d])])
                    e_cl_arr.append(e_arr [np.where(uni_barno==c[d])])
                    
                clr_posx.append(np.sum([a*b for a,b in zip(x_cl_arr,e_cl_arr)])/np.sum(e_cl_arr))
                clr_posy.append(np.sum([a*b for a,b in zip(y_cl_arr,e_cl_arr)])/np.sum(e_cl_arr))
                clr_eng.append(np.sum(e_cl_arr))
        clr_posx = list(flatten([arr.tolist() for arr in clr_posx]))
        clr_posy = list(flatten([arr.tolist() for arr in clr_posy]))
        clr_eng = list(flatten([arr.tolist() for arr in clr_eng]))

        cluster_data.loc[a,"ClusterposX_{0}".format(j)] =  clr_posx
        cluster_data.loc[a,"ClusterposY_{0}".format(j)] =  clr_posy
        cluster_data.loc[a,"Cluster_Eng_{0}".format(j)] =  clr_eng

cluster_data.to_csv("sample_cluster_data.csv", index = False, sep=',', encoding='utf-8')                    
       



    
#fig2 = plt.figure()
#xticks = np.arange(-5, 5, 1)
#yticks = np.arange(-5, 5, 1)
#plt.hist2d(xdata,ydata, bins=100)
#plt.xlabel('x')
#plt.ylabel('y')
#cbar = plt.colorbar()
#cbar.ax.set_ylabel('Counts')   
#plt.xticks(xticks)
#plt.yticks(yticks)
#
#plt.show() 
#    
#station_data.to_csv("10k_ps.csv", index = False, sep=',', encoding='utf-8')
#   
#    
#
#        
#
#
#        
#    
#    
#    
#    
#
#        
#        
#            
#
#        
#    
#
##scatter plots
#for j in range (0,20):
#    x=muon_data.iloc[j, 10]
#    y=muon_data.iloc[j, 11]
#    z=muon_data.iloc[j, 12]
#    stnno=muon_data.iloc[j, 13]
#    barno=muon_data.iloc[j, 15]
#    yticks = np.arange(0, 12, 1)
#    plt.scatter(z,stnno)
#    plt.yticks(yticks)
#    plt.show()
#
#fig = plt.figure(figsize=(12,10))
#ax = Axes3D(fig)
#ax.scatter(z, y,x)
#
#ax.set_xlabel('$x$', fontsize=20)
#ax.set_ylabel('$y$', fontsize=20)
#ax.set_zlabel('$z$', fontsize=20)
#plt.show()
#
#plt.scatter(z,barno)
#plt.show()
#
#
#stationnodata=np.concatenate( muon_data.iloc[:, 13].to_list(), axis=0 )
##ll=np.concatenate( l, axis=0 )
#
#
##histograms
#fig = plt.figure() 
#ax = fig.add_subplot(111)
#
#xticks = np.arange(-40, 160, 5)
#
##ax.set_xlim(-40,0)
#
#plt.suptitle("Z hits histogram", fontsize = 20)
#ax.set_xlabel('Z position (cm)', fontsize = 13)
##ax.set_ylabel('No of hits per event', fontsize = 13)
#
#ax.xaxis.set_minor_locator(AutoMinorLocator())
#ax.yaxis.set_minor_locator(AutoMinorLocator())
#
#	# Specify tick parameters
#ax.tick_params(axis = 'both', which = 'major', labelsize = 10)
#ax.tick_params(axis = 'both', which = 'minor', labelsize = 10)
#ax.tick_params(which='both', width=1)
#ax.tick_params(which='major', length=14)
#ax.tick_params(which='minor', length=7)
#ax.tick_params(which = 'both', direction = 'out')
#ax.grid(which = 'minor', alpha = 0.3)
#ax.grid(which = 'major', alpha = 0.7)
#
#ax.hist(zdata,bins=50)
#fig.set_size_inches(15, 9, forward=True)
#ax.set_xticks(xticks)
#plt.show()
#
#
#
##l=muon_data.iloc[:, 3].to_list()
##ll=np.concatenate( l, axis=0 )
##ls=[x.shape for x in l]
##ls=[i[0] for i in ls]
##ls.index(2)
##counter=collections.Counter(ls)
##print(counter)
