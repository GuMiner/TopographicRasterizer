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
TODO
## Generating 3D models
TODO
## 3D Printing
TODO