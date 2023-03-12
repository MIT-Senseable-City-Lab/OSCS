# -*- coding: utf-8 -*-
"""
Created on Wed Feb 16 12:02:37 2022

@author: anwan
"""
import pandas as pd
import numpy as np
import math
import random
# from sklearn.model_selection import train_test_split
from sklearn.model_selection import GridSearchCV
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LinearRegression
from sklearn.linear_model import ElasticNet
from sklearn.svm import SVR
from sklearn.ensemble import RandomForestRegressor
from pygam import LinearGAM, s
from lightgbm import LGBMRegressor
from keras.layers import Dense
from keras.models import Sequential
import joblib



def arbi_group(cs_df, num_grp):
    num_chunk = math.floor(cs_df.shape[0]/num_grp)
    cs_df['group'] = 0
    for i in range(0, num_grp):
        if i < num_grp - 1:
            cs_df['group'].iloc[i*num_chunk:(i+1)*num_chunk] = i
        else:
            cs_df['group'].iloc[i*num_chunk::] = i
    return cs_df['group']



def master_bysensor(cs_dir, plt_id):
    for i in range(0, 1): # 5 sensors
        cs_1min_dir = cs_dir + 'SyncAQ_cs0' + str(i+1) + '1min.csv'
        cs_5min_dir = cs_dir + 'SyncAQ_cs0' + str(i+1) + '5min.csv'
        cs_10min_dir = cs_dir + 'SyncAQ_cs0' + str(i+1) + '10min.csv'
        cs_30min_dir = cs_dir + 'SyncAQ_cs0' + str(i+1) + '30min.csv'
        cs_60min_dir = cs_dir + 'SyncAQ_cs0' + str(i+1) + '60min.csv'
        cs_1min = pd.read_csv(cs_1min_dir)
        cs_5min = pd.read_csv(cs_5min_dir)
        cs_10min = pd.read_csv(cs_10min_dir)
        cs_30min = pd.read_csv(cs_30min_dir)
        cs_60min = pd.read_csv(cs_60min_dir)
        intv = 0
        best_lrrsquared = 0
        best_elnetrsquared = 0
        best_gamrsquared = 0
        best_svrrsquared = 0
        best_rfrsquared = 0
        best_gbtrsquared = 0
        best_annrsquared = 0
        lr_rsquared = np.zeros((100, 5))
        elnet_rsquared = np.zeros((100, 5))
        gam_rsquared = np.zeros((100, 5))
        svr_rsquared = np.zeros((100, 5))
        rf_rsquared = np.zeros((100, 5))
        gbt_rsquared = np.zeros((100, 5))
        ann_rsquared = np.zeros((100, 5))
        lr_rmse = np.zeros((100, 5))
        elnet_rmse = np.zeros((100, 5))
        gam_rmse = np.zeros((100, 5))
        svr_rmse = np.zeros((100, 5))
        rf_rmse = np.zeros((100, 5))
        gbt_rmse = np.zeros((100, 5))
        ann_rmse = np.zeros((100, 5))
        # data preparation
        for cs_df in [cs_1min, cs_5min, cs_10min, cs_30min, cs_60min]: # 5 aggregation intervals
            xvar_df = cs_df[['PM25', 'gas_op2_w','tmpf','dwpf','relh','mslp']]
            xvar_df['group'] = arbi_group(xvar_df, 20)
            yvar_df = cs_df[['PM25FEM','NO2']]
            xvar_df['log_PM25'] = np.log(xvar_df['PM25'])
            yvar_df['log_PM25FEM'] = np.log(yvar_df['PM25FEM'])
            yvar_df['log_NO2'] = np.log(yvar_df['NO2'])
            include_n_group1 = random.sample (range(0,20), 17)
            if plt_id == 0:  #PM2.5
                xtraintest = xvar_df[['log_PM25','tmpf','dwpf','relh','mslp']].loc[xvar_df['group'].isin(include_n_group1)]
                ytraintest = yvar_df['log_PM25FEM'].loc[xvar_df['group'].isin(include_n_group1)]
                xval = xvar_df[['log_PM25','tmpf','dwpf','relh','mslp']].loc[xvar_df['group'].isin(include_n_group1)==False]
                yval = yvar_df['log_PM25FEM'].loc[xvar_df['group'].isin(include_n_group1)==False]
            elif plt_id == 1:   #NO2
                xtraintest = xvar_df[['gas_op2_w','tmpf','dwpf','relh','mslp']].loc[xvar_df['group'].isin(include_n_group1)]
                ytraintest = yvar_df['log_NO2'].loc[xvar_df['group'].isin(include_n_group1)]
                xval = xvar_df[['gas_op2_w','tmpf','dwpf','relh','mslp']].loc[xvar_df['group'].isin(include_n_group1)==False]
                yval = yvar_df['log_NO2'].loc[xvar_df['group'].isin(include_n_group1)==False]
            # if plt_id == 0:        # PM2.5
            #     xtraintest, xval, ytraintest, yval = train_test_split(
            #         xvar_df[['log_PM25','tmpf','dwpf','relh','mslp']],
            #         yvar_df['log_PM25FEM'], test_size = 0.2)
            # elif plt_id == 1:      # NO2
            #     xtraintest, xval, ytraintest, yval = train_test_split(
            #         xvar_df[['gas_op2_w','tmpf','dwpf','relh','mslp']],
            #         yvar_df['log_NO2'], test_size = 0.2)                      
            xscaler = StandardScaler()
            yscaler = StandardScaler()
            sc_xval = xscaler.fit_transform(xval)
            arr_yval = np.array(yval)
            sc_yval = yscaler.fit_transform(arr_yval.reshape(len(arr_yval), 1))
            # Hyperparam tuning (Elastic Net, SVR, RF, gbt)
            elnet = elnet_tune(xval, yval)        
            svr = svr_tune(sc_xval, sc_yval)  # Use normalized data for SVR
            rf = rf_tune(xval, yval)
            gbt = gbt_tune(xval, yval)
            xtraintest['group'] = arbi_group(xtraintest, 20)
            for j in range(0, 100):
                include_n_group2 = random.sample (range(0,20) , 17)
                if plt_id == 0:  #PM2.5
                    xtrain = xtraintest[['log_PM25','tmpf','dwpf','relh','mslp']].loc[xtraintest['group'].isin(include_n_group2)]
                    ytrain = ytraintest['log_PM25FEM'].loc[xtraintest['group'].isin(include_n_group2)]
                    xtest = xtraintest[['log_PM25','tmpf','dwpf','relh','mslp']].loc[xtraintest['group'].isin(include_n_group2)==False]
                    ytest = ytraintest['log_PM25FEM'].loc[xtraintest['group'].isin(include_n_group2)==False]
                elif plt_id == 1:   #NO2
                    xtrain = xtraintest[['gas_op2_w','tmpf','dwpf','relh','mslp']].loc[xtraintest['group'].isin(include_n_group2)]
                    ytrain = ytraintest.loc[xtraintest['group'].isin(include_n_group2)]
                    xtest = xtraintest[['gas_op2_w','tmpf','dwpf','relh','mslp']].loc[xtraintest['group'].isin(include_n_group2)==False]
                    ytest = ytraintest.loc[xtraintest['group'].isin(include_n_group2)==False]
                sc_xtrain = xscaler.fit_transform(xtrain)
                sc_xtest = xscaler.fit_transform(xtest)
                sc_ytrain = np.array(ytrain)
                sc_ytrain = yscaler.fit_transform(sc_ytrain.reshape(len(sc_ytrain), 1))
                cs_regr = LinearRegression().fit(xtrain, ytrain)
                cs_elnet = ElasticNet(**elnet.best_params_).fit(xtrain, ytrain)
                cs_gam = LinearGAM(s(0)+s(1)+s(2)+s(3)+s(4), 
                                    fit_intercept=True).fit(xtrain, ytrain)
                cs_svr = SVR(**svr.best_params_).fit(sc_xtrain, sc_ytrain)
                cs_rf = RandomForestRegressor(**rf.best_params_).fit(xtrain, ytrain)
                cs_gbt = LGBMRegressor(**gbt.best_params_).fit(xtrain, ytrain)
                cs_ann = Sequential()
                cs_ann.add(Dense(4, input_dim=5, kernel_initializer='normal',
                                  activation='sigmoid'))
                cs_ann.add(Dense(4, activation='sigmoid'))
                cs_ann.add(Dense(4, activation='sigmoid'))
                cs_ann.add(Dense(1, kernel_initializer='normal'))
                cs_ann.compile(loss='mean_squared_error', optimizer='adam')
                cs_ann.fit(sc_xtrain, sc_ytrain, epochs=200, batch_size=32, verbose=0) # Use normalized data for ANN
                # Calculate performance
                lr_yprd = cs_regr.predict(xtest)
                lr_rsquared[j, intv] = np.corrcoef(ytest.values, lr_yprd)[1,0]**2
                lr_rmse[j, intv] = np.sqrt(((ytest.values-lr_yprd)**2).mean())
                elnet_yprd = cs_elnet.predict(xtest)
                elnet_rsquared[j, intv] = np.corrcoef(ytest.values, elnet_yprd)[1,0]**2
                elnet_rmse[j, intv] = np.sqrt(((ytest.values-elnet_yprd)**2).mean())
                gam_yprd = cs_gam.predict(xtest)
                gam_rsquared[j, intv] = np.corrcoef(ytest.values, gam_yprd)[1,0]**2
                gam_rmse[j, intv] = np.sqrt(((ytest.values-gam_yprd)**2).mean())
                svr_yprd = cs_svr.predict(sc_xtest)  # Use normalized data
                svr_yprd = yscaler.inverse_transform(svr_yprd.reshape(len(svr_yprd),1))
                svr_rsquared[j, intv] = np.corrcoef(ytest.values, np.squeeze(svr_yprd))[1,0]**2
                svr_rmse[j, intv] = np.sqrt(((ytest.values-np.squeeze(svr_yprd))**2).mean())
                rf_yprd = cs_rf.predict(xtest)
                rf_rsquared[j, intv] = np.corrcoef(ytest.values, rf_yprd)[1,0]**2
                rf_rmse[j, intv] = np.sqrt(((ytest.values-rf_yprd)**2).mean())                
                gbt_yprd = cs_gbt.predict(xtest)
                gbt_rsquared[j, intv] = np.corrcoef(ytest.values, gbt_yprd)[1,0]**2
                gbt_rmse[j, intv] = np.sqrt(((ytest.values-gbt_yprd)**2).mean())  
                ann_yprd = cs_ann.predict(sc_xtest)  # Use normalized data
                ann_yprd = yscaler.inverse_transform(ann_yprd.reshape(len(ann_yprd),1))
                ann_rsquared[j, intv] = np.corrcoef(ytest.values, np.squeeze(ann_yprd))[1,0]**2
                ann_rmse[j, intv] = np.sqrt(((ytest.values-np.squeeze(ann_yprd))**2).mean())
                # Preserve best model
                if lr_rsquared[j, intv] > best_lrrsquared:
                    best_lrrsquared = lr_rsquared[j, intv]
                    best_lr = cs_regr
                if elnet_rsquared[j, intv] > best_elnetrsquared:
                    best_elnetrsquared = elnet_rsquared[j, intv]
                    best_elnet = cs_elnet
                if gam_rsquared[j, intv] > best_gamrsquared:
                    best_gamrsquared = gam_rsquared[j, intv]
                    best_gam = cs_gam
                if svr_rsquared[j, intv] > best_svrrsquared:
                    best_svrrsquared = svr_rsquared[j, intv]
                    best_svr = cs_svr
                if rf_rsquared[j, intv] > best_rfrsquared:
                    best_rfrsquared = rf_rsquared[j, intv]
                    best_rf = cs_rf
                if gbt_rsquared[j, intv] > best_gbtrsquared:
                    best_gbtrsquared = gbt_rsquared[j, intv]
                    best_gbt = cs_gbt
                if ann_rsquared[j, intv] > best_annrsquared:
                    best_annrsquared = ann_rsquared[j, intv]
                    best_ann = cs_ann                  
            # Output best model
            joblib.dump(best_lr,'D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Models2//lr_cs0'+str(i+1)+'_intv0'+str(intv+1)+'_plt'+str(plt_id))
            joblib.dump(best_elnet,'D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Models2//elnet_cs0'+str(i+1)+'_intv0'+str(intv+1)+'_plt'+str(plt_id))
            joblib.dump(best_gam,'D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Models2//gam_cs0'+str(i+1)+'_intv0'+str(intv+1)+'_plt'+str(plt_id))
            joblib.dump(best_svr,'D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Models2//svr_cs0'+str(i+1)+'_intv0'+str(intv+1)+'_plt'+str(plt_id))
            joblib.dump(best_rf,'D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Models2//rf_cs0'+str(i+1)+'_intv0'+str(intv+1)+'_plt'+str(plt_id))
            joblib.dump(best_gbt,'D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Models2//gbt_cs0'+str(i+1)+'_intv0'+str(intv+1)+'_plt'+str(plt_id))
            best_ann.save('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Models2//ann_cs0'+str(i+1)+'_intv0'+str(intv+1)+'_plt'+str(plt_id)+'.h5')
            intv += 1            
        # Output performance tables
        lr_r2df = pd.DataFrame(lr_rsquared, columns=['1min','5min','10min','30min','60min'])
        lr_rmsedf = pd.DataFrame(lr_rmse, columns=['1min','5min','10min','30min','60min'])
        lr_r2df.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//lr_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rsquared.csv')
        lr_rmsedf.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//lr_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rmse.csv')
        elnet_r2df = pd.DataFrame(elnet_rsquared, columns=['1min','5min','10min','30min','60min'])
        elnet_rmsedf = pd.DataFrame(elnet_rmse, columns=['1min','5min','10min','30min','60min'])
        elnet_r2df.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//elnet_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rsquared.csv')
        elnet_rmsedf.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//elnet_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rmse.csv')
        gam_r2df = pd.DataFrame(gam_rsquared, columns=['1min','5min','10min','30min','60min'])
        gam_rmsedf = pd.DataFrame(gam_rmse, columns=['1min','5min','10min','30min','60min'])
        gam_r2df.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//gam_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rsquared.csv')
        gam_rmsedf.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//gam_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rmse.csv')
        svr_r2df = pd.DataFrame(svr_rsquared, columns=['1min','5min','10min','30min','60min'])
        svr_rmsedf = pd.DataFrame(svr_rmse, columns=['1min','5min','10min','30min','60min'])
        svr_r2df.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//svr_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rsquared.csv')
        svr_rmsedf.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//svr_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rmse.csv')
        rf_r2df = pd.DataFrame(rf_rsquared, columns=['1min','5min','10min','30min','60min'])
        rf_rmsedf = pd.DataFrame(rf_rmse, columns=['1min','5min','10min','30min','60min'])
        rf_r2df.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//rf_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rsquared.csv')
        rf_rmsedf.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//rf_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rmse.csv')
        gbt_r2df = pd.DataFrame(gbt_rsquared, columns=['1min','5min','10min','30min','60min'])
        gbt_rmsedf = pd.DataFrame(gbt_rmse, columns=['1min','5min','10min','30min','60min'])
        gbt_r2df.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//gbt_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rsquared.csv')
        gbt_rmsedf.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//gbt_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rmse.csv')
        ann_r2df = pd.DataFrame(ann_rsquared, columns=['1min','5min','10min','30min','60min'])
        ann_rmsedf = pd.DataFrame(ann_rmse, columns=['1min','5min','10min','30min','60min'])
        ann_r2df.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//ann_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rsquared.csv')
        ann_rmsedf.to_csv('D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//Master_Calib_Perfs2//ann_cs0'+str(i+1)+'_plt'+str(plt_id)+'_rmse.csv')        



#def mater_allsensor():
#    return


def elnet_tune(xvalid, yvalid):
    elnet_params = {'alpha': np.logspace(-3, 2, num=6),
                    'l1_ratio': np.linspace(0, 1, num=10)}
    best_elnet = GridSearchCV(ElasticNet(), elnet_params, refit=False, scoring='neg_root_mean_squared_error').fit(xvalid, yvalid)
    return best_elnet


def svr_tune(xvalid, yvalid):
    # svr_params = {'C':[1, 10]}
    svr_params = {'C':[1, 10, 100, 1000],
                  'gamma':[0.5, 0.1, 0.01, 0.001],
                  'epsilon':[0.1, 0.2, 0.3, 0.5],
                  'kernel':['rbf','poly','sigmoid']}
    best_svr = GridSearchCV(SVR(), svr_params, refit=True, scoring='neg_root_mean_squared_error').fit(xvalid, yvalid)
    return best_svr


def rf_tune(xvalid, yvalid):
    # rf_params = {'n_estimators':[100, 1000]}
    rf_params = {'n_estimators':[200, 500, 1000],
                  'max_features':['sqrt','log2'],
                  'min_samples_split':[2, 4, 8],
                  'min_samples_leaf':[0.001, 0.01, 0.1]}
    best_rf = GridSearchCV(RandomForestRegressor(), rf_params, refit=False, scoring='neg_root_mean_squared_error').fit(xvalid, yvalid)        
    return best_rf


def gbt_tune(xvalid, yvalid):
    # gbt_params = {'booster': ['gbtree'],
    #                'objective': ['reg:squarederror'],
    #                'eta': [0.001, 0.003]}
    gbt_params = {'learning_rate': [0.0001, 0.001, 0.01, 0.1],
                   'num_leaves': [10, 20, 30, 100],
                   'max_depth':[3, 5, 9, 15],
                   'max_bin':[10, 20, 40, 80],
                   'min_data_in_leaf':[50, 100, 200, 500],
                   'reg_alpha':[0, 0.1, 0.3],
                   'reg_lambda':[0, 0.1, 0.3]}   
    best_gbt = GridSearchCV(LGBMRegressor(), gbt_params, refit=False, scoring='neg_root_mean_squared_error').fit(xvalid, yvalid)        
    return best_gbt



cs_dir = 'D://MIT Research Docs//CityScannerCalibration_2108//AirQualityData//NYC_Calibration_2109//SyncAQData//'
#master_bysensor(cs_dir, 0)
master_bysensor(cs_dir, 1)






