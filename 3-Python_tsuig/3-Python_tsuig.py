#!/usr/bin/env python
# coding: utf-8

# In[1]:


#Glen Tsui
#400201284
#Python 3.6.0

#import needed libraries
import numpy as np
import open3d as o3d

#read point cloud xyz file
print("Testing IO for point cloud...")
pcd = o3d.io.read_point_cloud("demofile2dx4.xyz", format = 'xyz')

#print points
print(pcd)
print(np.asarray(pcd.points))

#initialization of point variables and array for line appends
pt = 0
pt1 = 0
po = 0
lines = []

#adds points and connects lines along revolution plane
#for 12 full revolutions
for x in range(12):
    #for 128 measurements per revolution
    for pt in range(128):
        #increment points
        pt1 = pt + 1
        #if on the last measurement, append to the first point again
        if(pt == 127):
            pt1 = 0
        #connect lines between each point
        lines.append([pt+po,(pt1)+po])
    #next plane
    po += 128

#reinitialize variables
pt = 0
po = 0
do = 128
# connect lines
#for 11 connections in between 12 full revolutions
for x in range(11):
    #for 128 measurements per revolution
    for pt in range(128):
        #connect lines between each plane
        lines.append([pt+po,pt+do+po])
        #next plane
    po += 128

#evaluate line sets using array of points and lines
line_set = o3d.geometry.LineSet(points = o3d.utility.Vector3dVector(np.asarray(pcd.points)), lines = o3d.utility.Vector2iVector(lines))

#display results on open3d
o3d.visualization.draw_geometries([line_set])


# In[ ]:




