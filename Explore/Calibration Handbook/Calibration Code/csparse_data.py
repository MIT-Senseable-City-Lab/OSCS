# -*- coding: utf-8 -*-
"""
Created on Wed Oct 19 17:10:11 2022

@author: anwan
"""
import pandas as pd
import os

# Please customize your own file directory here.
cs_dir = os.path.abspath('D://MIT Research Docs//CityScannerAMS_2210//amsday1//nyc0305//QUEUE')
headers = ['flagID','deviceID', 'timestamp', 'latitude', 'longitude', 'PM1', 'PM25', 'PM4','PM10',\
    'numPM0', 'numPM1', 'numPM2', 'numPM4', 'numPM10', 'PartSize','temperature', 'humidity',\
    'gas_op1_w', 'gas_op1_r', 'gas_op2_w', 'gas_op2_r', 'noise']
cs_df = pd.DataFrame()
for root, dirs, files in os.walk(cs_dir):
    for file in files:
        temp_tab = pd.read_csv(cs_dir+'//'+file, names=headers)
        cs_df = pd.concat([cs_df, temp_tab], axis=0)
cs_df = cs_df[cs_df['flagID']==0]
cs_df = cs_df.drop(['flagID'], axis=1)
cs_df.to_csv(cs_dir+'//'+'compile_raw.csv', index=None)

