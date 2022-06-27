# Geophyphox-Excursion
This repository contains:
- notebooks for the geophyphox excursion 2022 
- notebook to split excursion data by student id
- test data
- synthetic data

## Overview

### Pre Processing (done by supervisors)
The csv-file from the MQTT-Server will contain students measurement data. Using the "Split Dataset" notebook the dataframe can be split up using the
student_id. That way each student will process only their data and later combine it with the others.

### Processing
Each student processes their data using the "Magnetics_processing" notebook. Processed data should be uploaded to a folder, which will later be provided to students used to combine all data.  
Processing steps:
- Outlier detection
- Time correction
- (Altitude correction)
- Base station correction
- Anomaly calculation
- Data export

### Transformation
Using the "Magnetic_transformation" notebook all students data will be combined into one dataframe. Some synthetic data showing how transformations should work will be included.
Which will be used for:
- Interpolation (linear spline, nearest neighbor)   
- Map visualization   
- Tranformation (upward continuation and reduce to pole)
- Profile extraction

### Modeling and Inversion
In this part students will use the profile-data they extracted in the transformation notebook and will use inversion methods to model the anomaly
Includes:
- Magnetic sphere anomaly formula
- Effect of inclination and profile orientation
- Fitting sphere to measured profile data

