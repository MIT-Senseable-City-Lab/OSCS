# -*- coding: utf-8 -*-
"""
Created on Wed Oct 19 17:10:11 2022

@author: anwan
"""
import pandas as pd
import os

# Please customize your own file directory here.
cs_dir = os.path.abspath('D://MIT Research Docs//CityScannerAMS_2210//amsday1//nyc0305//QUEUE')
headers = ['flagID','deviceID', 'timestamp', 'latitude', 'longitude', 'PM1', 'PM25', 'PM10',\
    'bin0', 'bin1', 'bin2', 'bin3', 'bin4', 'bin5', 'bin6', 'bin7', 'bin8', 'bin9',\
    'bin10', 'bin11', 'bin12', 'bin13', 'bin14', 'bin15', 'bin16', 'bin17', 'bin18',\
    'bin19', 'bin20', 'bin21', 'bin22', 'bin23', 'flowrate', 'countglitch', 'laser_status',\
    'temperature_opc', 'humidity_opc', 'data_is_valid', 'temperature', 'humidity',\
    'ambient_IR', 'object_IR', 'gas_op1_w', 'gas_op1_r', 'gas_op2_w', 'gas_op2_r', 'noise']
cs_df = pd.DataFrame()
for root, dirs, files in os.walk(cs_dir):
    for file in files:
        temp_tab = pd.read_csv(cs_dir+'//'+file, names=headers)
        cs_df = pd.concat([cs_df, temp_tab], axis=0)
cs_df = cs_df[cs_df['flagID']==0]
cs_df = cs_df.drop(['flagID'], axis=1)
cs_df.to_csv(cs_dir+'//'+'compile_raw.csv', index=None)

