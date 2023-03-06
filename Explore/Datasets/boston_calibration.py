# -*- coding: utf-8 -*-
"""
Created on Mon Feb 13 13:28:25 2023

@author: anwan
"""

import pandas as pd
import numpy as np
from sklearn.model_selection import GridSearchCV
from sklearn.ensemble import RandomForestRegressor


def boston_calib(cs_dir, plt_id):
    if plt_id == 0:
        cs03_df = pd.read_csv(cs_dir + 'SyncAQ_cs03_plt0_intv01.csv')
        cs05_df = pd.read_csv(cs_dir + 'SyncAQ_cs05_plt0_intv01.csv')
        cs03_df = cs03_df.loc[(cs03_df['PM25']<50) & (cs03_df['OFFSET_AEROSOL']<50) & (cs03_df['relh']<85)]
        cs05_df = cs05_df.loc[(cs05_df['PM25']<50) & (cs05_df['OFFSET_AEROSOL']<50) & (cs05_df['relh']<85)]
        cs03_xvar = cs03_df[['PM25','tmpf','dwpf','relh','mslp']]
        cs05_xvar = cs05_df[['PM25','tmpf','dwpf','relh','mslp']]
        cs03_yvar = cs03_df[['AEROSOL','OFFSET_AEROSOL']]
        cs05_yvar = cs05_df[['AEROSOL','OFFSET_AEROSOL']]
        cs03_xvar['log_PM25'] = np.log(cs03_xvar['PM25'])
        cs05_xvar['log_PM25'] = np.log(cs05_xvar['PM25'])
        cs03_yvar['log_PM25REF'] = np.log(cs03_yvar['OFFSET_AEROSOL']*257.1+0.3861)
        cs05_yvar['log_PM25REF'] = np.log(cs05_yvar['OFFSET_AEROSOL']*257.1+0.3861)
        cs03_x = cs03_xvar[['log_PM25','tmpf','dwpf','relh','mslp']]
        cs05_x = cs05_xvar[['log_PM25','tmpf','dwpf','relh','mslp']]
        cs03_y = cs03_yvar['log_PM25REF']
        cs05_y = cs05_yvar['log_PM25REF']
    elif plt_id == 1:
        cs03_df = pd.read_csv(cs_dir + 'SyncAQ_cs03_plt1_intv01.csv')
        cs05_df = pd.read_csv(cs_dir + 'SyncAQ_cs05_plt1_intv01.csv') 
        cs03_df = cs03_df.loc[(cs03_df['no2']<100) & (cs03_df['no2']>0) & (cs03_df['relh']<85)]
        cs05_df = cs05_df.loc[(cs05_df['no2']<100) & (cs05_df['no2']>0) & (cs05_df['relh']<85)]
        cs03_xvar = cs03_df[['gas_op2_w','tmpf','dwpf','relh','mslp']]
        cs05_xvar = cs05_df[['gas_op2_w','tmpf','dwpf','relh','mslp']]
        cs03_yvar = cs03_df[['no2','nox']]
        cs05_yvar = cs05_df[['no2','nox']]
        cs03_yvar['log_NO2REF'] = np.log(cs03_yvar['no2'])
        cs05_yvar['log_NO2REF'] = np.log(cs05_yvar['no2'])
        cs03_x = cs03_xvar[['gas_op2_w','tmpf','dwpf','relh','mslp']]
        cs05_x = cs05_xvar[['gas_op2_w','tmpf','dwpf','relh','mslp']]
        cs03_y = cs03_yvar['log_NO2REF']
        cs05_y = cs05_yvar['log_NO2REF']
    rf_params = {'n_estimators':[200, 500, 1000],
                 'max_features':['sqrt','log2'],
                 'min_samples_split':[2, 4, 8],
                 'min_samples_leaf':[0.001, 0.01, 0.1]}
    best_rf03 = GridSearchCV(RandomForestRegressor(), rf_params, refit=True, scoring='neg_root_mean_squared_error').fit(cs03_x, cs03_y)        
    best_rf05 = GridSearchCV(RandomForestRegressor(), rf_params, refit=True, scoring='neg_root_mean_squared_error').fit(cs05_x, cs05_y) 
    rf03_yprd = best_rf03.predict(cs03_x)
    rf03_r = np.corrcoef(np.exp(cs03_y.values), np.exp(rf03_yprd))[1,0]  
    rf03_rmse = np.sqrt(((np.exp(cs03_y.values)-np.exp(rf03_yprd))**2).mean()) 
    rf05_yprd = best_rf05.predict(cs05_x)
    rf05_r = np.corrcoef(np.exp(cs05_y.values), np.exp(rf05_yprd))[1,0]  
    rf05_rmse = np.sqrt(((np.exp(cs05_y.values)-np.exp(rf05_yprd))**2).mean())    
    # Mobile calibration 
    cs03_mobile = pd.read_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//Tufts_MobileCalib_2202//City Scanner Data//s3_sync.csv')
    cs05_mobile = pd.read_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//Tufts_MobileCalib_2202//City Scanner Data//s5_sync.csv')
    if plt_id == 0:
        cs03_mobile['log_PM25'] = np.log(cs03_mobile['PM25'])
        cs05_mobile['log_PM25'] = np.log(cs05_mobile['PM25']) 
        cs03_xmobile = cs03_mobile[['log_PM25','tmpf','dwpf','relh','mslp']]
        cs05_xmobile = cs05_mobile[['log_PM25','tmpf','dwpf','relh','mslp']]
        cs03_mobile['Calib_logPM25'] = best_rf03.predict(cs03_xmobile)
        cs05_mobile['Calib_logPM25'] = best_rf05.predict(cs05_xmobile)
        cs03_mobile['Calib_PM25'] = np.exp(cs03_mobile['Calib_logPM25'])
        cs05_mobile['Calib_PM25'] = np.exp(cs05_mobile['Calib_logPM25'])
    elif plt_id == 1:
        cs03_xmobile = cs03_mobile[['gas_op2_w','tmpf','dwpf','relh','mslp']]
        cs05_xmobile = cs05_mobile[['gas_op2_w','tmpf','dwpf','relh','mslp']]
        cs03_mobile['Calib_logNO2'] = best_rf03.predict(cs03_xmobile)
        cs05_mobile['Calib_logNO2'] = best_rf03.predict(cs05_xmobile)  
        cs03_mobile['Calib_NO2'] = np.exp(cs03_mobile['Calib_logNO2'])
        cs05_mobile['Calib_NO2'] = np.exp(cs05_mobile['Calib_logNO2'])   
    cs03_mobile.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//Tufts_MobileCalib_2202//s3_calibrated_01.csv')
    cs05_mobile.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//Tufts_MobileCalib_2202//s5_calibrated_01.csv')    
    return rf03_r, rf03_rmse, rf05_r, rf05_rmse


# bsrf03_r, bsrf03_rmse, bsrf05_r, bsrf05_rmse = boston_calib('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//Tufts_MobileCalib_2202//Sync_AQData//', 0)
bsrf03_r, bsrf03_rmse, bsrf05_r, bsrf05_rmse = boston_calib('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//Tufts_MobileCalib_2202//Sync_AQData//', 1)





