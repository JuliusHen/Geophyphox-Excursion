# Packages needed for notebooks:
In order for the notebooks to work properly, you will need to install some additional packages besides the packages included in the anaconda distribution.

Creating a virtual enviroment with all the necessary packages is the best way to do so. For this you open the Anaconda prompt and type the following:
    conda create -n geophyphox_2022 jupyter notebook numpy matplotlib pandas pathlib pyproj scipy cartopy ipympl 
after completion you will need to install mpl_point_clicker via pip. Enter the following commands in the Anaconda Prompt:
    conda activate geophyphox_2022
    pip install mpl_point_clicker

Now you can start Jupyter Notebook by typing "jupyter notebook" in the Anaconda prompt (If you activated your created enviroment with conda activate geophyphox_2022)