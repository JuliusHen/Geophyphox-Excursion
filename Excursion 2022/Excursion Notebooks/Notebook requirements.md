# Packages needed for notebooks:

In order for the notebooks to work properly, you will need to install some additional packages besides the packages included in the anaconda distribution.

Creating a virtual enviroment with all the necessary packages is the best way to do so. For this you open the Anaconda prompt and type the following :
    conda create -n Test numpy matplotlib pandas pathlib pyproj scipy cartopy ipympl 
after completion you will need to install mpl_point_clicker via pip:
    pip install mpl_point_clicker