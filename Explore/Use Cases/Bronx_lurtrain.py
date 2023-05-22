# -*- coding: utf-8 -*-
"""
Created on Mon Jun 20 14:29:09 2022

@author: anwan

"""
import pandas as pd
import numpy as np
from sklearn.model_selection import GridSearchCV
from sklearn.model_selection import train_test_split
from sklearn.model_selection import KFold
from sklearn.linear_model import LinearRegression
from sklearn.ensemble import RandomForestRegressor
from lightgbm import LGBMRegressor
import shap
import joblib

brx_trainlu = '...//BronxFallPMGrid_MedianTrainRstr.csv'
brx_prdlu = '...//BronxFallGrid_LU.csv'
lga_meteo = '...//DailyMeteo_Sep2Dec.csv'

brx_NO2trainlu = '...//BronxFallNO2Grid_MedianTrainRstr.csv'


# pm_linperf, best_pmlin = brx_linregr(brx_trainlu)


def rf_hyptune(lur_dir, plt_id = 0):
    brx_lu = pd.read_csv(lur_dir)
    brxlu_df = brx_lu.loc[:,'median_tmp':]
    brxlu_df = brxlu_df.dropna()
    if plt_id == 0: # PM2.5
        xvar_df = brxlu_df.drop(['log_PM25','log_CalibP','Calib_PM25','median_drc','median_vsb','median_fee'], axis=1)
        yvar_df = brxlu_df[['log_CalibP','Calib_PM25']]
        xtraintest, xval, ytraintest, yval = train_test_split(xvar_df, 
                                                      yvar_df['log_CalibP'], 
                                                      test_size = 0.2)
    elif plt_id == 1: # NO2
        xvar_df = brxlu_df.drop(['log_CalibN','Calib_NO2','median_drc','median_vsb','median_fee'], axis=1)
        yvar_df = brxlu_df[['log_CalibN','Calib_NO2']]
        xtraintest, xval, ytraintest, yval = train_test_split(xvar_df, 
                                                      yvar_df['log_CalibN'], 
                                                      test_size = 0.2)
    rf_params = {'n_estimators':[200, 500, 1000],
                  'max_features':['sqrt','log2'],
                  'min_samples_split':[2, 4, 8],
                  'min_samples_leaf':[0.001, 0.01, 0.1]}
    best_rf = GridSearchCV(RandomForestRegressor(), 
                           rf_params, 
                           refit=False, 
                           scoring='neg_root_mean_squared_error').fit(xval, yval)           
    return best_rf, xtraintest, ytraintest    
# pm_rf, pm_xtraintest, pm_ytraintest = rf_hyptune(brx_trainlu)
# joblib.dump(pm_lgb,'...//PM_RF_WStd')
# pm_xtraintest.to_csv('...//PM_RF_xWStd.csv', index=False)
# pm_ytraintest.to_csv('...//PM_RF_yWStd.csv', index=False)   

# no2_rf, no2_xtraintest, no2_ytraintest = rf_hyptune(brx_NO2trainlu, 1)
# joblib.dump(no2_lgb,'...//NO2_LGBT_MedianRStr')
# no2_xtraintest.to_csv('...//NO2_LGBT_xMedianRStr.csv', index=False)
# no2_ytraintest.to_csv('...//NO2_LGBT_yMedianRStr.csv', index=False)  


def lgb_hyptune(lur_dir, plt_id = 0):
    brx_lu = pd.read_csv(lur_dir)
    brxlu_df = brx_lu.loc[:,'log_CalibN':]
    brxlu_df = brxlu_df.dropna()
    if plt_id == 0: # PM2.5
        xvar_df = brxlu_df.drop(['log_PM25','log_CalibP','Calib_PM25','median_drc','median_vsb','median_fee'], axis=1)
        yvar_df = brxlu_df[['log_CalibP','Calib_PM25']]
        xtraintest, xval, ytraintest, yval = train_test_split(xvar_df, 
                                                      yvar_df['log_CalibP'], 
                                                      test_size = 0.2)
    elif plt_id == 1: # NO2 
        xvar_df = brxlu_df.drop(['log_CalibN','Calib_NO2','median_drc','median_vsb','median_fee'], axis=1)
        # xvar_df = brxlu_df.drop(['log_CalibN','Calib_NO2','median_drc','median_vsb','median_fee'], axis=1)
        yvar_df = brxlu_df[['log_CalibN','Calib_NO2']]
        xtraintest, xval, ytraintest, yval = train_test_split(xvar_df, 
                                                      yvar_df['log_CalibN'], 
                                                      test_size = 0.2)
    # xtraintest = pd.DataFrame(xtraintest, columns=xvar_df.columns)   
    lgb_params = {'boosting_type': ['gbdt'],
                  'objective': ['regression'],
                  'metric': ['rmse'],
                  'num_iterations': [100],
                  'learning_rate': [0.0001, 0.001, 0.01, 0.05, 0.1],
                  'num_leaves': [5, 10, 15, 20, 50],
                  'max_depth':[3, 5, 7, 9],
                  'max_bin':[10, 20, 40, 80],
                  'min_data_in_leaf':[5, 10, 20, 50, 100],
                  'reg_alpha':[0, 0.1, 0.3, 0.7],
                  'reg_lambda':[0, 0.1, 0.3, 0.7]}   
    best_lgb = GridSearchCV(LGBMRegressor(), 
                            lgb_params, 
                            refit=False, 
                            scoring='neg_root_mean_squared_error').fit(xval, yval)     
    return best_lgb, xtraintest, ytraintest # xscaler, yscaler

# pm_lgb, pm_xtraintest, pm_ytraintest = lgb_hyptune(brx_trainlu)
# joblib.dump(pm_lgb,'...//PM_LGBT_MedianRStr')
# pm_xtraintest.to_csv('...//PM_LGBT_xMedianRStr.csv', index=False)
# pm_ytraintest.to_csv('...//PM_LGBT_yMedianRStr.csv', index=False)

# no2_lgb, no2_xtraintest, no2_ytraintest = lgb_hyptune(brx_NO2trainlu, 1)
# joblib.dump(no2_lgb,'...//NO2_LGBT_MedianRStr')
# no2_xtraintest.to_csv('...//NO2_LGBT_xMedianRStr.csv', index=False)
# no2_ytraintest.to_csv('...//NO2_LGBT_yMedianRStr.csv', index=False)


def nyclu_train(valid_lgb, xtraintest, ytraintest):
    ten_fold = KFold(n_splits = 10, random_state = 1, shuffle = True)
    ten_fold_data = ten_fold.split(xtraintest, ytraintest)
    best_lgbr2 = 0
    lgb_perf = np.zeros((10, 2))
    for k, (train, test) in enumerate(ten_fold_data):
        lur_lgb = LGBMRegressor(**valid_lgb.best_params_).fit(
                                                        xtraintest.iloc[train, :], 
                                                        ytraintest.iloc[train, 0])
        lgb_yprd = lur_lgb.predict(xtraintest.iloc[test, :])
        lgb_yprd_exp = np.exp(lgb_yprd)
        lgb_ytest_exp = np.exp(ytraintest.iloc[test, 0].values)
        lgb_perf[k, 0] = np.corrcoef(lgb_ytest_exp, lgb_yprd_exp)[1,0]**2
        lgb_perf[k, 1] = np.sqrt(((lgb_ytest_exp - lgb_yprd_exp)**2).mean()) 
        # lgb_ytest = ytraintest.iloc[test,0].values
        # lgb_perf[k, 0] = np.corrcoef(lgb_ytest, lgb_yprd)[1,0]**2
        # lgb_perf[k, 1] = np.sqrt(((lgb_ytest - lgb_yprd)**2).mean())        
        if lgb_perf[k, 0] > best_lgbr2:
            best_lgb = lur_lgb
            best_lgbr2 = lgb_perf[k, 0]
    print('Average Pseudo-Rsquared:%.2f' %lgb_perf[k, 0].mean())
    print('Average RMSE:%.2f' %lgb_perf[k, 1].mean())
    print('Best Pseudo-Rsquared:%.2f'%best_lgbr2)
    lur_perf = pd.DataFrame(lgb_perf, columns = ['Pseudo-R2', 'RMSE'])
    return lur_perf, best_lgb

# pm_lgb = joblib.load('...//PM_LGBT_MedianRStr')
# pm_xtraintest = pd.read_csv('...//PM_LGBT_xMedianRStr.csv')
# pm_ytraintest = pd.read_csv('...//PM_LGBT_yMedianRStr.csv')
# pmlur_perf, best_pmlgb = nyclu_train(pm_lgb, pm_xtraintest, pm_ytraintest)
# joblib.dump(best_pmlgb, '...//PM_BestLGBT_MedianRStr')

# no2_lgb = joblib.load('...//NO2_LGBT_MedianRStr')
# no2_xtraintest = pd.read_csv('...//NO2_LGBT_xMedianRStr.csv')
# no2_ytraintest = pd.read_csv('...//NO2_LGBT_yMedianRStr.csv')
# no2lur_perf, best_no2rf = nyclu_train(no2_lgb, no2_xtraintest, no2_ytraintest)
# joblib.dump(best_no2lgb, '...//NO2_BestLGBT_MedianRStr')


def nyclu_metpredict(lur_model, prdlu_dir, meteo_dir, plt_id = 0):
    brx_lu =  pd.read_csv(prdlu_dir)
    meteo_df = pd.read_csv(meteo_dir)
    xvardf = brx_lu.loc[:,'StDist':]
    xvardf = xvardf.dropna()
    # Iterate through all dates
    for index,row in meteo_df.iterrows():
        xvardf[['tmpf','dwpf','relh','drct','sknt','mslp','vsby','feel']] = row[1:]  
        xvar_list = list(xvardf.columns)
        xvar_nulist = xvar_list[-8:] + xvar_list[:-8]
        xvardf = xvardf[xvar_nulist]
        xvardf = xvardf.drop(['drct','vsby','feel'], axis=1)
        if plt_id == 0:
            log_prd = lur_model.predict(xvardf)
            brx_lu['Log_PrdPM'] = log_prd
            brx_lu['Prd_PM25'] = np.exp(log_prd)
            brx_prd = brx_lu[['PM_fall_La','Log_PrdPM','Prd_PM25']]            
        elif plt_id == 1:
            log_prd = lur_model.predict(xvardf)
            brx_lu['Log_PrdNO2'] = log_prd
            brx_lu['Prd_NO2'] = np.exp(log_prd)
            brx_prd = brx_lu[['PM_fall_La','Log_PrdNO2','Prd_NO2']]
        brx_prd.to_csv('...//DailyNO2Predictions_Median//BronxFallGrid_Prd'+str(plt_id)+'_'+row['valid']+'.csv', index=False)
        xvardf = xvardf.drop(['tmpf','dwpf','relh','sknt','mslp'], axis=1)
    # return brx_prd
# best_pmlgb = joblib.load('...//PM_BestLGBT_MedianRStr')
# nyclu_metpredict(best_pmlgb, brx_prdlu, lga_meteo)

# best_no2lgb = joblib.load('...//NO2_BestLGBT_MedianRStr')
# nyclu_metpredict(best_no2lgb, brx_prdlu, lga_meteo, 1)

def nyclu_nometpred (lur_model, prdlu_dir, plt_id = 0):
    brx_lu =  pd.read_csv(prdlu_dir)
    xvardf = brx_lu.loc[:,'StDist':]
    xvardf = xvardf.dropna()
    if plt_id == 0:
        log_prd = lur_model.predict(xvardf)
        brx_lu['Log_PrdPM'] = log_prd
        brx_lu['Prd_PM25'] = np.exp(log_prd)
        brx_prd = brx_lu[['PM_fall_La','Log_PrdPM','Prd_PM25']]            
    elif plt_id == 1:
        log_prd = lur_model.predict(xvardf)
        brx_lu['Log_PrdNO2'] = log_prd
        brx_lu['Prd_NO2'] = np.exp(log_prd)
        brx_prd = brx_lu[['PM_fall_La','Log_PrdNO2','Prd_NO2']]        
    brx_prd.to_csv('...//BronxFallGrid_PrdnoMeteo.csv', index=False)


# best_pmlgb = joblib.load('...//PM_BestLGBT_Mean')
# nyclu_nometpred(best_pmlgb, brx_prdlu)
    
def lur_shapsumm(lur_model, xtraintest):
    shap_mdl = shap.TreeExplainer(lur_model)
    shap_val = shap_mdl.shap_values(xtraintest)
    shap.summary_plot(shap_val, xtraintest, show=True)
    
best_pmlgb = joblib.load('...//PM_BestLGBT_MedianRStr')
pm_xtraintest = pd.read_csv('...//PM_LGBT_xMedianRStr.csv')
lur_shapsumm(best_pmlgb, pm_xtraintest)

# best_no2lgb = joblib.load('...//NO2_BestLGBT_MedianRStr')
# no2_xtraintest = pd.read_csv('...//NO2_LGBT_xMedianRStr.csv')
# lur_shapsumm(best_no2rf, no2_xtraintest)


def lur_shapsingle(lur_model, prdlu_dir, meteo_dir):
    brx_lu = pd.read_csv(prdlu_dir)
    meteo_df = pd.read_csv(meteo_dir)
    xvardf = brx_lu.loc[:,'StDist':]
    xvardf = xvardf.dropna()
    xvardf[['tmpf','dwpf','relh','drct','sknt','mslp','vsby','feel']] = meteo_df.iloc[-1, 1:]
    xvar_list = list(xvardf.columns)
    xvar_nulist = xvar_list[-8:] + xvar_list[:-8]
    xvardf = xvardf[xvar_nulist]
    xvardf = xvardf.drop(['drct','vsby','feel'], axis=1)
    shap.initjs()
    shap_mdl = shap.TreeExplainer(lur_model)
    shap_val = shap_mdl.shap_values(xvardf)    
    shap.force_plot(shap_mdl.expected_value, shap_val[brx_lu['PM_fall_La']==48869,:], xvardf.loc[brx_lu['PM_fall_La']==48869,:], matplotlib=True)


# best_pmlgb = joblib.load('...//PM_BestLGBT_MedianRStr')
# lur_shapsingle(best_pmlgb, brx_prdlu, lga_meteo)






