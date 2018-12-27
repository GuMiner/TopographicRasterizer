# Example
For this example, we'll generate a 3D map of Mt Baker, WA.

## Acquiring Data
To get the topographic data, we will use the USGS National Map.
This topographic data is free and reasonable quality for our purposes.

* Navigate to https://viewer.nationalmap.gov/basic/
![Find the website](./E1.PNG)
* Move the map to 48.7691 Latitute, -121.8154 Longitude (Mt Baker, WA)
* Select 'Use Map' and 'Current Extent'
![Selecting map area](./E3.PNG)
* Select 'Elevation Products (3DEP)', 'Contours (1:24,000-scale)', and 'Shapefile'
![Selecting elevation data](./E2.PNG)
* Hit 'Find Products'. There should be a single result
![Search result](./E4.PNG)
* Hit 'Thumbprint' to verify that Mount Baker' is within the selected area
![Mount Baker](./E5.PNG)
* Download and extract the zip file.

## Preprocessing Data
This downloaded data covers more regional terrain than we are interested in and isn't in the GeoJSON format. We're going to preprocess this data using [QGIS](https://qgis.org/en/site/index.html).

* Open QGIS
![QGIS](./E6.PNG)
* Add a new vector layer, selecting the 'Directory' input option and 'Shape' folder
![QGIS2](./E7.PNG)
![QGIS3](./E8.PNG)
* Add a new temporary scratch layer, naming it 'CropLayer' with 'Polygon' type
![QGIS4](./E9.PNG)
![QGIS5](./E10.PNG)
* In the top toolbar, select the add-region button
* 
![QGIS6](./E11.PNG)
* Click three points to select a square and right-click to add the square around Mount Baker. This doesn't need to be an exact square, but try to make it as square as you can. Once complete, the display should look something like this:
![QGIS7](./E12.PNG)
* Navigate to the Clip tool such that all the geometry outside of the square is removed.

![QGIS8](./E13.PNG)
![QGIS9](./E14.PNG)
* Once you're left with the geometry, we're ready to output the data as GeoJSON.
* First, we'll reduce the number of properties to only the feature we are interested in (elevation). To do so, right-click on the temporary region and select 'Properties'

![QGIS10](./E14.1.PNG)
* Navigate to the below selection, hit the pencil to edit, and remove all fields except for 'CONTOURELE' **You'll need to remember this field name later.**
![QGIS11](./E14.2.PNG)
* Right-click again and select 'Make Permanent', saving the layer as a GeoJSON file.
* 
![QGIS12](./E15.PNG)
![QGIS12](./E16.PNG)

## Rasterizing Data
### Generating Rasters
To rasterize the data, we'll use this application. To start, download and unzip the latest release from TODO.

There are a number of tool options, but we'll use the default settings. This will cause the application to generate a 10x10 grid of 800x800 pixel images. These images will be stored in a 'rasters' folder in the same directory where we run it.

Open a command prompt and run the following, replacing the file name with the GeoJSON file exported earlier:
    
    .\ContourTiler.exe MtBaker_file.geojson --Feature CONTOURELE
    
Notably, we exported the data with the elevation in the 'CONTOURELE' feature instead of the default 'Elevation' feature

ContourTiler will load the data, output statistics, and open up a rendered preview of the terrain.

![ContourTiler](./C1.PNG)

The console lists a series of commands you can run using this display, such as typing **L** to render contours, **C** to render a color spectrum, etc.

We don't need to zoom to any particular feature, so press **P** to start bulk processing. Wait for the output console to say **Tiling and rasterization done!**

In the meantime, image files will be generated and stored in the **rasters** folder next to the application.

Once complete, you can close the application but should keep the console window open.

### Combinging Rasters
By default, the rasters are separated into many files for efficient loading in video games. To 3D print a model, we have to combine them all together.

To do so, run the following PowerShell script in the same console window

    TODO copy over script

## Generating 3D models
Once we have a single image file, we can load it into TODO to convert it into a 3D model.

TODO

## 3D Printing
With the 3D model, we can generate code to send it to the 3D printer.

TODO