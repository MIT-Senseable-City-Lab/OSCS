# -*- coding: utf-8 -*-
"""
Created on Fri Jun 17 15:36:59 2022

@author: anwan
"""

import geopandas as gpd
import pandas as pd
from shapely.geometry import Point, Polygon, shape
import matplotlib.pyplot as plt

bronx_grid = 'D://MIT Research Docs//NYC_LUR_2206//Mapping Files//Projected Land Use Surfaces//Bronx//NYCCASFallGrid_Bronx.shp'
nyc_aadt = 'D://MIT Research Docs//NYC_LUR_2206//Mapping Files//Projected Land Use Surfaces//Bronx//AADT_Bronx.shp'
nyc_road = 'D://MIT Research Docs//NYC_LUR_2206//Mapping Files//Projected Land Use Surfaces//Bronx//Roads_Bronx.shp'
nyc_busroute = 'D://MIT Research Docs//NYC_LUR_2206//Mapping Files//Projected Land Use Surfaces//Bronx//BusRoute_Bronx.shp'
nyc_truckroute = 'D://MIT Research Docs//NYC_LUR_2206//Mapping Files//Projected Land Use Surfaces//Bronx//TruckRoute_Bronx.shp'
nyc_popu = 'D://MIT Research Docs//NYC_LUR_2206//Mapping Files//Projected Land Use Surfaces//Bronx//BlockGroup_Bronx.shp'
nyc_pluto = 'D://MIT Research Docs//NYC_LUR_2206//Mapping Files//Projected Land Use Surfaces//Bronx//MapPluto_Bronx.shp'
nyc_facility = 'D://MIT Research Docs//NYC_LUR_2206//Mapping Files//Projected Land Use Surfaces//Bronx//Facilities_Bronx.shp'
bronx_commcook = 'D://MIT Research Docs//NYC_LUR_2206//Mapping Files//Projected Land Use Surfaces//Bronx//Bronx_CommCooking.shp'
# brxgrid_df should have the same dimension as bronx_grid
brxgrid_df = pd.read_csv('D://MIT Research Docs//NYC_LUR_2206//Python Codes//BronxFallGrid.csv')
# brxgrid_df = brxgrid_df.drop(brxgrid_df.columns[range(1,25)], axis=1)

##### Area and length calculation #####

def feature_calc(layer, buff_row, feat_name):
    layer_idx = layer.intersects(buff_row)
    if feat_name == 'Green':
        layer_slice = layer.loc[layer_idx]
        layer_slice = layer_slice.loc[layer_slice['LandUse']=='09']
        feat_val = layer_slice['LotArea'].sum(skipna=True)
    else:
        layer_slice = layer.loc[layer_idx]
        feat_val = layer_slice[feat_name].sum(skipna=True)
    return feat_val

def grab_len_area(grid_dir, aadt_dir, road_dir, busroute_dir, truckroute_dir, popu_dir, pluto_dir, buffer_size):
    grid = gpd.read_file(grid_dir)
    buff = grid.buffer(buffer_size)
    aadt = gpd.read_file(aadt_dir)
    road = gpd.read_file(road_dir)
    busroute = gpd.read_file(busroute_dir)
    truckroute = gpd.read_file(truckroute_dir)
    popu = gpd.read_file(popu_dir)
    pluto = gpd.read_file(pluto_dir)
    # Length
    street = road.loc[road['rw_type'] == 1]
    highway = road.loc[road['rw_type'] == 2]
    ramp = road.loc[road['rw_type'] == 9]
    uni_street = street.geometry.unary_union
    uni_highway = highway.geometry.unary_union
    uni_ramp = ramp.geometry.unary_union
    uni_busroute = busroute.geometry.unary_union
    uni_truckroute = truckroute.geometry.unary_union
    street_length = buff.intersection(uni_street).length
    highway_length = buff.intersection(uni_highway).length + buff.intersection(uni_ramp).length 
    busroute_length = buff.intersection(uni_busroute).length
    truckroute_length = buff.intersection(uni_truckroute).length
    buff_aadt = buff.apply(lambda row: feature_calc(aadt, row, 'AADT')) 
    buff_trkaadt = buff.apply(lambda row: feature_calc(aadt, row, 'TruckAADT'))
    # Area
    bld_area = buff.apply(lambda row: feature_calc(pluto, row, 'BldgArea'))
    resd_area = buff.apply(lambda row: feature_calc(pluto, row, 'ResArea'))
    resd_unit = buff.apply(lambda row: feature_calc(pluto, row, 'UnitsRes'))
    inds_area = buff.apply(lambda row: feature_calc(pluto, row, 'ComArea'))
    comm_area = buff.apply(lambda row: feature_calc(pluto, row, 'FactryArea'))
    recr_area = buff.apply(lambda row: feature_calc(pluto, row, 'Green'))
    cats_num = buff.apply(lambda row: feature_calc(pluto, row, 'CATS_BBLSu'))
    bnchmrk_btu = buff.apply(lambda row: feature_calc(pluto, row, 'NYCBenchma'))
    buff_popu = buff.apply(lambda row: feature_calc(popu, row, 'total_popu'))
    return street_length, highway_length, busroute_length, truckroute_length, buff_aadt, buff_trkaadt, bld_area, resd_area, resd_unit, inds_area, comm_area, recr_area, cats_num, bnchmrk_btu, buff_popu

# brxgrid_df['StLength_500'], brxgrid_df['HwLength_500'], brxgrid_df['BsRtLength_500'], brxgrid_df['TrkRtLength_500'],\
# brxgrid_df['AADT_500'], brxgrid_df['TruckAADT_500'], brxgrid_df['BldArea_500'],\
# brxgrid_df['RsdArea_500'], brxgrid_df['RsdUnit_500'], brxgrid_df['IndArea_500'],\
# brxgrid_df['ComArea_500'], brxgrid_df['RecArea_500'], brxgrid_df['CATS_500'],\
# brxgrid_df['Bnchmrk_500'], brxgrid_df['TotalPopu_500'] = grab_len_area(bronx_grid, nyc_aadt, nyc_road, nyc_busroute, nyc_truckroute, nyc_popu, nyc_pluto, 500)
# print('Done 500 area & length')

###### Distance calculation #####

def grab_dist(grid_dir, road_dir, busroute_dir, truckroute_dir, facility_dir):
    grid = gpd.read_file(grid_dir)
    road = gpd.read_file(road_dir)
    facility = gpd.read_file(facility_dir)
    bsroute = gpd.read_file(busroute_dir)
    trkroute = gpd.read_file(truckroute_dir)
    street = road.loc[road['rw_type'] == 1]
    highway = road.loc[road['rw_type'] == 2]
    busdepot = facility.loc[facility['facsubgrp'].str.contains('BUS DEPOTS')]
    port = facility.loc[facility['facsubgrp'].str.contains('PORTS AND FERRY')]
    railyard = facility.loc[facility['facsubgrp'].str.contains('RAIL YARDS')]
    wasteproc = facility.loc[facility['facsubgrp'].str.contains('SOLID WASTE')]
    watertreat = facility.loc[facility['facsubgrp'].str.contains('WASTEWATER AND POLLUTION')]
    airport = facility.loc[facility['facsubgrp'].str.contains('AIRPORTS AND HELIPORTS')]
    uni_street = street.geometry.unary_union
    uni_highway = highway.geometry.unary_union
    uni_bsroute = bsroute.geometry.unary_union
    uni_trkroute = trkroute.geometry.unary_union
    uni_busdepot = busdepot.geometry.unary_union
    uni_port = port.geometry.unary_union
    uni_railyard = railyard.geometry.unary_union
    uni_wasteproc = wasteproc.geometry.unary_union
    uni_watertreat = watertreat.geometry.unary_union
    uni_airport = airport.geometry.unary_union
    st_dist = grid.distance(uni_street)
    hw_dist = grid.distance(uni_highway)
    bsrt_dist = grid.distance(uni_bsroute)
    trkrt_dist = grid.distance(uni_trkroute)
    bsdp_dist = grid.distance(uni_busdepot)
    port_dist = grid.distance(uni_port)
    rlyd_dist = grid.distance(uni_railyard)
    wstproc_dist = grid.distance(uni_wasteproc)
    wttrt_dist = grid.distance(uni_watertreat)
    aport_dist = grid.distance(uni_airport)
    st_dist = st_dist.replace(0,1)
    hw_dist = hw_dist.replace(0,1)
    bsrt_dist = bsrt_dist.replace(0,1)
    trkrt_dist = trkrt_dist.replace(0,1)
    bsdp_dist = bsdp_dist.replace(0,1)
    port_dist = port_dist.replace(0,1)
    rlyd_dist = rlyd_dist.replace(0,1)
    wstproc_dist = wstproc_dist.replace(0,1)
    wttrt_dist = wttrt_dist.replace(0,1)
    aport_dist = aport_dist.replace(0,1)
    return st_dist, hw_dist, bsrt_dist, trkrt_dist, bsdp_dist, port_dist, rlyd_dist, wstproc_dist, wttrt_dist, aport_dist


##### Number of facilities calculation #####

def geoseries_count(geoseries_row):
    if geoseries_row.is_empty:
        count = 0
    elif geoseries_row.geom_type == 'Point':
        count = 1
    else:
        count = len(geoseries_row)    
    return count

def grab_num(grid_dir, facility_dir, buffer_size):
    # nad83_crs = {'init': 'epsg:6538'}
    grid = gpd.read_file(grid_dir)
    facility = gpd.read_file(facility_dir)
    # Create buffer around grid
    buff = grid.buffer(buffer_size)
    bus_depot = facility.loc[facility['facsubgrp'].str.contains('BUS DEPOTS')]
    intersect_busdepot = buff.intersection(bus_depot.geometry.unary_union)
    num_busdepot = intersect_busdepot.apply(lambda row: geoseries_count(row))
    port = facility.loc[facility['facsubgrp'].str.contains('PORTS AND FERRY')]
    intersect_port = buff.intersection(port.geometry.unary_union)
    num_port = intersect_port.apply(lambda row: geoseries_count(row))
    railyard = facility.loc[facility['facsubgrp'].str.contains('RAIL YARDS')]
    intersect_railyard = buff.intersection(railyard.geometry.unary_union)
    num_railyard = intersect_railyard.apply(lambda row: geoseries_count(row))
    waste_proc = facility.loc[facility['facsubgrp'].str.contains('SOLID WASTE')]
    intersect_wasteproc = buff.intersection(waste_proc.geometry.unary_union)
    num_wasteproc = intersect_wasteproc.apply(lambda row: geoseries_count(row))
    water_treat = facility.loc[facility['facsubgrp'].str.contains('WASTEWATER AND POLLUTION')]
    intersect_watertreat = buff.intersection(water_treat.geometry.unary_union)
    num_watertreat = intersect_watertreat.apply(lambda row: geoseries_count(row))
    airport = facility.loc[facility['facsubgrp'].str.contains('AIRPORTS AND HELIPORTS')]
    intersect_airport = buff.intersection(airport.geometry.unary_union)
    num_airport = intersect_airport.apply(lambda row: geoseries_count(row))    
    return num_busdepot, num_port, num_railyard, num_wasteproc, num_watertreat, num_airport

# brxgrid_df['StDist'], brxgrid_df['HwDist'], brxgrid_df['BsRtDist'], brxgrid_df['TrkRtDist'],\
# brxgrid_df['BsDpDist'], brxgrid_df['PortDist'], brxgrid_df['RlydDist'],\
# brxgrid_df['WstProcDist'], brxgrid_df['WtTrtDist'], brxgrid_df['AportDist'] = grab_dist(bronx_grid, nyc_road, nyc_busroute, nyc_truckroute, nyc_facility)
# print('Finished distance calculation')
# brxgrid_df.to_csv('D://MIT Research Docs//NYC_LandUseRegression//Python Codes//BronxFallGrid_LU1.csv')

# brxgrid_df['StLength_50'], brxgrid_df['HwLength_50'], brxgrid_df['BsRtLength_50'], brxgrid_df['TrkRtLength_50'],\
# brxgrid_df['AADT_50'], brxgrid_df['TruckAADT_50'], brxgrid_df['BldArea_50'],\
# brxgrid_df['RsdArea_50'], brxgrid_df['RsdUnit_50'], brxgrid_df['IndArea_50'],\
# brxgrid_df['ComArea_50'], brxgrid_df['RecArea_50'], brxgrid_df['CATS_50'],\
# brxgrid_df['Bnchmrk_50'], brxgrid_df['TotalPopu_50'] = grab_len_area(bronx_grid, nyc_aadt, nyc_road, nyc_busroute, nyc_truckroute, nyc_popu, nyc_pluto, 50)
# print('Finished area & length calculation 50')

# brxgrid_df['StLength_100'], brxgrid_df['HwLength_100'], brxgrid_df['BsRtLength_100'], brxgrid_df['TrkRtLength_100'],\
# brxgrid_df['AADT_100'], brxgrid_df['TruckAADT_100'], brxgrid_df['BldArea_100'],\
# brxgrid_df['RsdArea_100'], brxgrid_df['RsdUnit_100'], brxgrid_df['IndArea_100'],\
# brxgrid_df['ComArea_100'], brxgrid_df['RecArea_100'], brxgrid_df['CATS_100'],\
# brxgrid_df['Bnchmrk_100'], brxgrid_df['TotalPopu_100'] = grab_len_area(bronx_grid, nyc_aadt, nyc_road, nyc_busroute, nyc_truckroute, nyc_popu, nyc_pluto, 100)
# print('Finished area & length calculation 100')
# brxgrid_df['StLength_250'], brxgrid_df['HwLength_250'], brxgrid_df['BsRtLength_250'], brxgrid_df['TrkRtLength_250'],\
# brxgrid_df['AADT_250'], brxgrid_df['TruckAADT_250'], brxgrid_df['BldArea_250'],\
# brxgrid_df['RsdArea_250'], brxgrid_df['RsdUnit_250'], brxgrid_df['IndArea_250'],\
# brxgrid_df['ComArea_250'], brxgrid_df['RecArea_250'], brxgrid_df['CATS_250'],\
# brxgrid_df['Bnchmrk_250'], brxgrid_df['TotalPopu_250'] = grab_len_area(bronx_grid, nyc_aadt, nyc_road, nyc_busroute, nyc_truckroute, nyc_popu, nyc_pluto, 250)
# print('Finished area & length calculation 250')
# brxgrid_df.to_csv('D://MIT Research Docs//NYC_LandUseRegression//Python Codes//BronxFallGrid_LU2.csv')

# brxgrid_df['StLength_500'], brxgrid_df['HwLength_500'], brxgrid_df['BsRtLength_500'], brxgrid_df['TrkRtLength_500'],\
# brxgrid_df['AADT_500'], brxgrid_df['TruckAADT_500'], brxgrid_df['BldArea_500'],\
# brxgrid_df['RsdArea_500'], brxgrid_df['RsdUnit_500'], brxgrid_df['IndArea_500'],\
# brxgrid_df['ComArea_500'], brxgrid_df['RecArea_500'], brxgrid_df['CATS_500'],\
# brxgrid_df['Bnchmrk_500'], brxgrid_df['TotalPopu_500'] = grab_len_area(bronx_grid, nyc_aadt, nyc_road, nyc_busroute, nyc_truckroute, nyc_popu, nyc_pluto, 500)
# print('Finished area & length calculation 500')
# brxgrid_df['StLength_1000'], brxgrid_df['HwLength_1000'], brxgrid_df['BsRtLength_1000'], brxgrid_df['TrkRtLength_1000'],\
# brxgrid_df['AADT_1000'], brxgrid_df['TruckAADT_1000'], brxgrid_df['BldArea_1000'],\
# brxgrid_df['RsdArea_1000'], brxgrid_df['RsdUnit_1000'], brxgrid_df['IndArea_1000'],\
# brxgrid_df['ComArea_1000'], brxgrid_df['RecArea_1000'], brxgrid_df['CATS_1000'],\
# brxgrid_df['Bnchmrk_1000'], brxgrid_df['TotalPopu_1000'] = grab_len_area(bronx_grid, nyc_aadt, nyc_road, nyc_busroute, nyc_truckroute, nyc_popu, nyc_pluto, 1000)
# print('Finished area & length calculation 1000')
# brxgrid_df.to_csv('D://MIT Research Docs//NYC_LandUseRegression//Python Codes//BronxFallGrid_LU3.csv')

# brxgrid_df['BusDepot_50'], brxgrid_df['Port_50'], \
# brxgrid_df['Railyard_50'], brxgrid_df['WasteProc_50'],\
# brxgrid_df['WaterTreat_50'], brxgrid_df['Airport_50'] = grab_num(bronx_grid, nyc_facility, 50)
# print('Finished facility counting 50')
# brxgrid_df['BusDepot_100'], brxgrid_df['Port_100'], \
# brxgrid_df['Railyard_100'], brxgrid_df['WasteProc_100'],\
# brxgrid_df['WaterTreat_100'], brxgrid_df['Airport_100'] = grab_num(bronx_grid, nyc_facility, 100)
# print('Finished facility counting 100')
# brxgrid_df['BusDepot_250'], brxgrid_df['Port_250'], \
# brxgrid_df['Railyard_250'], brxgrid_df['WasteProc_250'],\
# brxgrid_df['WaterTreat_250'], brxgrid_df['Airport_250'] = grab_num(bronx_grid, nyc_facility, 250)
# print('Finished facility counting 250')
# brxgrid_df['BusDepot_500'], brxgrid_df['Port_500'], \
# brxgrid_df['Railyard_500'], brxgrid_df['WasteProc_500'],\
# brxgrid_df['WaterTreat_500'], brxgrid_df['Airport_500'] = grab_num(bronx_grid, nyc_facility, 500)
# print('Finished facility counting 500')
# brxgrid_df['BusDepot_1000'], brxgrid_df['Port_1000'], \
# brxgrid_df['Railyard_1000'], brxgrid_df['WasteProc_1000'],\
# brxgrid_df['WaterTreat_1000'], brxgrid_df['Airport_1000'] = grab_num(bronx_grid, nyc_facility, 1000)
# print('Finished facility counting 1000')
# brxgrid_df.to_csv('D://MIT Research Docs//NYC_LUR_2206//Python Codes//BronxFallGrid_LU9.csv')

def grab_restaurant(grid_dir, restr_dir):
    grid = gpd.read_file(grid_dir)
    restaurant = gpd.read_file(restr_dir)
    uni_restr = restaurant.geometry.unary_union
    # restr_dist = grid.distance(uni_restr)
    # restr_dist = restr_dist.replace(0, 1)
    # buff50 = grid.buffer(50)
    # buff100 = grid.buffer(100)
    # buff250 = grid.buffer(250)
    buff500 = grid.buffer(500)
    buff1000 = grid.buffer(1000)
    # inter50_restr = buff50.intersection(uni_restr)
    # num50_restr = inter50_restr.apply(lambda row: geoseries_count(row))
    # inter100_restr = buff100.intersection(uni_restr)
    # num100_restr = inter100_restr.apply(lambda row: geoseries_count(row))
    # inter250_restr = buff250.intersection(uni_restr)
    # num250_restr = inter250_restr.apply(lambda row: geoseries_count(row))
    inter500_restr = buff500.intersection(uni_restr)
    num500_restr = inter500_restr.apply(lambda row: geoseries_count(row))
    inter1000_restr = buff1000.intersection(uni_restr)
    num1000_restr = inter1000_restr.apply(lambda row: geoseries_count(row))    
    # return restr_dist, num50_restr, num100_restr, num250_restr, num500_restr, num1000_restr
    return  num500_restr, num1000_restr

# brxgrid_df['RstrDist'], brxgrid_df['Restaurant_50'], brxgrid_df['Restaurant_100'],\
# brxgrid_df['Restaurant_250'], brxgrid_df['Restaurant_500'], brxgrid_df['Restaurant_1000'] = grab_restaurant(bronx_grid, bronx_commcook)
brxgrid_df['Restaurant_500'], brxgrid_df['Restaurant_1000'] = grab_restaurant(bronx_grid, bronx_commcook)
# brxgrid_df.to_csv('D://MIT Research Docs//NYC_LUR_2206//Python Codes//BronxFallGrid_LU8.csv')
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    