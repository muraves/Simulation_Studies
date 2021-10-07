#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue May  4 10:30:20 2021

@author: amrutha, UGent
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
import scipy
from scipy.stats import norm

#path to the root output of Muravessim default root file
filee="/home/amrutha/muraves/sim2/MuravesSim/build/MuravesSim_hitpdg.root"
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
#zdata=np.concatenate( muon_data.iloc[:, 12].to_list(), axis=0 )

#splitting the whole data in to station wise data
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
        station_data["station_{0}HitPDG".format(i)] = ""

        
        
for a in range (0,len(muon_data),1):
#for a in range (0,10,1):                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
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
        station_data.loc[a,"station_{0}HitPDG".format(b)] = muon_data.iloc[a,16] [item_index]
#    print(a)
    
    
for i in range (0,12):
    if (i<4) | (i>7):
        station_data["station_{0}totE".format(i)] = ""  
        

station_data=station_data[:].fillna("").apply(list)


        
for a in range (0,len(muon_data),1):  
#for a in range (0,10,1): 
    station_data.iloc[a,58]=sum(station_data.iloc[a,5])
#    print(a)           
    station_data.iloc[a,59]=sum(station_data.iloc[a,12])
    station_data.iloc[a,60]=sum(station_data.iloc[a,19])
    station_data.iloc[a,61]=sum(station_data.iloc[a,26])
    station_data.iloc[a,62]=sum(station_data.iloc[a,33])
    station_data.iloc[a,63]=sum(station_data.iloc[a,40])
    station_data.iloc[a,64]=sum(station_data.iloc[a,47])
    station_data.iloc[a,65]=sum(station_data.iloc[a,54])

       
#xdata=np.concatenate(station_data["station_0X"].to_list(), axis=0 )
#ydata=np.concatenate(station_data["station_0Y"].to_list(), axis=0 )

#Renumbering the scintillator bars considering the module no (1 or 0)
bar_col=[6,13,20,34,41,48]
col_no=[0,1,2,8,9,10]

for a in range (0,len(station_data)): 
    for i,j in zip (bar_col,col_no):
        bar_no=station_data.iloc[a, i]
#        print(bar_no)
        mod_no=station_data.iloc[a, i+1]
        mod_index = np.where(mod_no==1)
        for l in mod_index:
            bar_no[l]=bar_no[l]+32
            
station_data_3lyr=station_data.drop(station_data.columns[[23,24,25,26,27,28,29,51,52,53,54,55,56,57,61,65]], axis=1) 

#clustering
cluster_data=pd.DataFrame()
cluster_data["event_id"] = ""
for i in range (0,11):
    if (i<3) | (i>7):
        cluster_data["HitposX_{0}".format(i)] = ""
        cluster_data["HitposY_{0}".format(i)] = ""
        cluster_data["HitE_per_bar_{0}".format(i)] = ""
        cluster_data["HitBar_{0}".format(i)] = ""
        cluster_data["ClusterposX_{0}".format(i)] = ""
        cluster_data["ClusterposY_{0}".format(i)] = ""
        cluster_data["Cluster_Eng_{0}".format(i)] = ""
        cluster_data["N_Clusters_{0}".format(i)] = ""
        cluster_data["Cluster_Size_{0}".format(i)] = ""
        cluster_data["Cluster_HitPDG_{0}".format(i)] = ""

#give the correct bar col id for 3 layer data        
bar_col=[6,13,20,27,34,41]
col_no=[0,1,2,8,9,10]
 
for a in range (0,len(station_data_3lyr)): 
#for a in range (0,1): 
#    print(a)
    cluster_data.loc[a,"event_id"]=a
    for i,j in zip (bar_col,col_no):
        bar_no=station_data_3lyr.iloc[a, i]
        uni_barno=np.unique(bar_no)
#        print(uni_barno)
        cluster_data.loc[a,"HitBar_{0}".format(j)]=uni_barno
        
        x_arr=[]
        y_arr=[]
        e_arr=[]
        for b in uni_barno:
             bar_index = np.where(bar_no==b)
             if (j<3):   
                 x_arr.append(-52.35+(b*1.66))
                 y_arr.append(0)               
                 e_arr.append(np.sum(station_data_3lyr.iloc[a,i-1] [bar_index]))
             if (j==9):
                 x_arr.append(0)
                 y_arr.append(5.65+(b*1.66))            
                 e_arr.append(np.sum(station_data_3lyr.iloc[a,i-1] [bar_index]))
             if (j==8):
                 x_arr.append(0)
                 y_arr.append(1.65+(b*1.66))            
                 e_arr.append(np.sum(station_data_3lyr.iloc[a,i-1] [bar_index]))
             if (j==10):
                 x_arr.append(0)
                 y_arr.append(-2.85+(b*1.66))            
                 e_arr.append(np.sum(station_data_3lyr.iloc[a,i-1] [bar_index]))
             
        x_arr=np.array(x_arr)
        y_arr=np.array(y_arr)
        e_arr=np.array(e_arr)
        cluster_data.loc[a,"HitposX_{0}".format(j)]=x_arr
        cluster_data.loc[a,"HitposY_{0}".format(j)]=y_arr
        cluster_data.loc[a,"HitE_per_bar_{0}".format(j)]=e_arr
        bar_list=[list(group) for group in mit.consecutive_groups(uni_barno)] 
        #To get secondary hits per cluster
        pdg_list=[]
        for m in bar_list:
            pdg_min_li=[]
            for n in m:
                bar_index_pdg = np.where(bar_no==n)
                pdg_min_li.append(station_data_3lyr.iloc[a,i+2] [bar_index_pdg])
            flat_pdg_min_li = [item for sublist in pdg_min_li for item in sublist]
            pdg_list.append(flat_pdg_min_li)

        cluster_data.loc[a,"Cluster_HitPDG_{0}".format(j)]=pdg_list  
        
        
        cluster_data.loc[a,"N_Clusters_{0}".format(j)] = len(bar_list)
        cl_size=[]
        for cs in range (0,len(bar_list)):
            cl_size.append(len(bar_list[cs]))
        cluster_data.loc[a,"Cluster_Size_{0}".format(j)] = cl_size
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

        
#2D tracking
planex=[1,0,2]
planey=[9,8,10]
z=np.asarray([-26,0,25])
chi_sqr=[]
chi_sqr_min2=[]
min_of_chisqr2=[]
chi_sqr_no=[]
tr_eng=[]
tr_eng_no=[]
res_min=[]
res_no_track=[]
res_min2=[]
HitPDG_track=[]
HitPDG_track_2=[]
HitPDG_no=[]

muon_track=[]
others_track=[]

muon_track2=[]
others_track2=[]

muon_track_no=[]
others_track_no=[]

for n in range(0,len(cluster_data)):
#for n in range(244,255):
    print(n)
    xp=[]
    st_xp=[]
    eng_xp=[]
    HitPDG_xp=[]
    yp=[]
    st_yp=[]
    eng_yp=[]
    HitPDG_yp=[]
    for o1,o2 in zip (planex,planey):
        xp.append(cluster_data.loc[n,"ClusterposX_{0}".format(o1)])
        st_xp.append(cluster_data.loc[n,"Cluster_Size_{0}".format(o1)])
        eng_xp.append(cluster_data.loc[n,"Cluster_Eng_{0}".format(o1)])
        HitPDG_xp.append(cluster_data.loc[n,"Cluster_HitPDG_{0}".format(o1)])
        
        yp.append(cluster_data.loc[n,"ClusterposY_{0}".format(o2)])
        st_yp.append(cluster_data.loc[n,"Cluster_Size_{0}".format(o2)])
        eng_yp.append(cluster_data.loc[n,"Cluster_Eng_{0}".format(o2)])
        HitPDG_yp.append(cluster_data.loc[n,"Cluster_HitPDG_{0}".format(o2)])
        
    track_points=[]
    track_stp=[]
    track_eg=[]
    track_HitPDG=[]
    
    for no in range (0,3):
        stn_points=[]
        stn_stp=[]
        stn_eg=[]
        stn_HitPDG=[]
        for a1,a2,a3,a4 in zip(xp[no],st_xp[no],eng_xp[no],HitPDG_xp[no]):  
            for b1,b2,b3,b4 in zip(yp[no],st_yp[no],eng_yp[no],HitPDG_yp[no]):
                stn_points.append([a1,b1]) 
                stn_stp.append([a2,b2])
                stn_eg.append([a3,b3])
                stn_HitPDG.append([a4,b4])
                
        track_points.append(stn_points)
        track_stp.append(stn_stp)
        track_eg.append(stn_eg)
        track_HitPDG.append(stn_HitPDG)
        
    chi_tot=[]
    eng_tot=[]
    res_tot=[]
    HitPDG_tot=[]
 
    for e1,f1,g1,h1 in zip(track_points[0],track_stp[0],track_eg[0], track_HitPDG[0]):
        for e2,f2,g2,h2 in zip(track_points[1],track_stp[1],track_eg[1],track_HitPDG[1]):
            for e3,f3,g3,h3 in zip (track_points[2],track_stp[2],track_eg[2],track_HitPDG[2]):
                x=[e1[0],e2[0],e3[0]]
                y=[e1[1],e2[1],e3[1]]
                bar_x=[f1[0],f2[0],f3[0]]
                bar_x=np.asarray(bar_x)
                bar_y=[f1[1],f2[1],f3[1]]
                bar_y=np.asarray(bar_y)
                eng_x=[g1[0],g2[0],g3[0]]
                eng_y=[g1[1],g2[1],g3[1]]
                HitPDG_x=[h1[0],h2[0],h3[0]]
                HitPDG_y=[h1[1],h2[1],h3[1]]
                mx, bx = np.polyfit(z,x, 1)
                xfit=(mx*z)+bx
                unc_x=3.3/(bar_x*np.sqrt(12))
                resx=np.square(x-xfit)
                subx=np.square((x-xfit)/unc_x)
                ch_x=sum(subx)

                
                my, by = np.polyfit(z,y, 1)
                yfit=(my*z)+by
                unc_y=3.3/(bar_y*np.sqrt(12))
                resy=np.square(y-yfit)
                suby=np.square((y-yfit)/unc_y)
                ch_y=sum(suby)
                
                res_tot.append(sum(resx)+sum(resy))
                chi_tot.append(ch_x+ch_y)
                eng_tot.append(sum(eng_x)+sum(eng_y))
                HitPDG_tot.append(HitPDG_x+HitPDG_y)
    len_muons=[]
    len_others=[]
    for k in range (0,len(HitPDG_tot)):
        HitPDG_tot[k]=[item for sublist in HitPDG_tot[k] for item in sublist]
        muons=HitPDG_tot[k].count(-13)
        others=len(HitPDG_tot[k])-muons
        len_muons.append(muons)
        len_others.append(others)
        
                
                
    min_chi_tot=min(chi_tot)
    chi_sqr.append(min_chi_tot)
    chi_ind=chi_tot.index(min_chi_tot)
    tr_eng.append(eng_tot[chi_ind])
    res_min.append(res_tot[chi_ind])
    HitPDG_track.append(HitPDG_tot[chi_ind])
    muon_track.append( len_muons[chi_ind])
    others_track.append(len_others[chi_ind])
    
    
    if (len(chi_tot)!=1):
        min_of_chisqr2.append(min_chi_tot)
        min2_chi_tot=sorted(chi_tot)[1]
        chi_sqr_min2.append(min2_chi_tot)
        chi_ind2=chi_tot.index(min2_chi_tot)
        chi_tot.remove(min_chi_tot)
        chi_sqr_no.extend(chi_tot)
        muon_track2.append( len_muons[chi_ind2])
        others_track2.append( len_others[chi_ind2])
        
        len_muons.remove(len_muons[chi_ind])
        len_others.remove(len_others[chi_ind])
        
        muon_track_no.extend(len_muons)
        others_track_no.extend(len_others)
        
        HitPDG_track_2.append(HitPDG_tot[chi_ind2])
        HitPDG_tot.remove(HitPDG_tot[chi_ind])
        HitPDG_no.extend(HitPDG_tot)
        
        
        
        
        eng_tot.remove(eng_tot[chi_ind])
        tr_eng_no.extend(eng_tot)
        
        res_tot.remove(res_tot[chi_ind])
        res_min2.append(min(res_tot))
        res_no_track.extend(res_tot)

cluster_data['track_chi2'] = chi_sqr
cluster_data['track_residue'] = res_min
cluster_data['track_energy']=tr_eng

#give path to and and name for output file
cluster_data.to_csv("MURAVESSIM_cluster_data.csv", index = False, sep=',', encoding='utf-8')
print("finished!! Have a good day :)")
#plotting
#diff=np.asarray(chi_sqr_min2)-np.asarray(min_of_chisqr2)
##histograms
#fig = plt.figure() 
#ax = fig.add_subplot(111)
#
##xticks = np.arange(0, 20, 4)
##
##ax.set_xlim(0,20)
##
##ax.set_xticks(xticks)
#
##plt.suptitle("Z hits histogram", fontsize = 20)
#ax.set_xlabel('Residue', fontsize = 20)
##ax.set_ylabel('No of hits per event', fontsize = 13)
#
#ax.xaxis.set_minor_locator(AutoMinorLocator())
#ax.yaxis.set_minor_locator(AutoMinorLocator())
#
#	# Specify tick parameters
#ax.tick_params(axis = 'both', which = 'major', labelsize = 15)
#ax.tick_params(axis = 'both', which = 'minor', labelsize = 15)
#ax.tick_params(which='both', width=1)
#ax.tick_params(which='major', length=14)
#ax.tick_params(which='minor', length=7)
#ax.tick_params(which = 'both', direction = 'out')
##ax.grid(which = 'minor', alpha = 0.3)
#ax.grid(which = 'major', alpha = 0.7,linestyle='--',color='black')
#
#
##ax.hist(chi_sqr,alpha=0.5, bins=bins, color='brown',fill=True,normed=True)
#n, bins, patches = ax.hist(res_min2, 50, facecolor='green', alpha=0.5,fill=True)
#mu, sigma = scipy.stats.norm.fit(res_min2)
#no=int(len(res_min2))
##plt.title(r'$\mathrm{Histogram: chisquare\ 2nd-chisquare\ minimum;}\ No\ of\ tracks=%.3f,\ \mu=%.3f$' %(no,mu),fontsize = 20)
#plt.title(r'$\mathrm{Residue\ Histogram: 2nd\ min\ chi2\ tracks;}\ No\ of\ tracks=%.3f,\ \mu=%.3f$' %(no,mu),fontsize = 20)
#plt.yscale('log')
#fig.set_size_inches(16, 9, forward=True)
#plt.show()
#                
                
#
#fig = plt.figure()
#ax = Axes3D(fig)
#ax.scatter(z, x, y)
#print(z,y,x)
#plt.show()
