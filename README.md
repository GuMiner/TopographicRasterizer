# TopographicRasterizer
Efficiently rasterizes topographic data into images for general use.

## Purpose
This application enables easily converting topographic data into a series of images for game development or for 3D printing. 

## Workflow
1. Export your topographic data as GeoJSON.
2. Run this application, selecting which regions to rasterize.
3. (optional) Run the provided PowerShell scripts to post-process the output data into a single image.

## Downloads
TODO create 

### Export
[QGIS](https://www.qgis.org/en/site/) is an excellent geographical software tool for importing topographic data from the [National Map](https://viewer.nationalmap.gov/advanced-viewer/) or elsewhere. However, the rasterization functionality of this tool is too limited for the generation of 3D prints.

The example section uses QGIS to load in a publiclly-available topographic map and export it as GeoJSON.

### Run 
TODO write how to run the tool and available options.

### Post-Process
*TopographicRasterer* outputs a series of 2D tiled PNG images. The height of the region is stored in each pixel as a 16-bit value, using the R and G (MSB) channels.

The 8-bit B channel is free for your own usage. For game development, this channel can be used to specify terrain types. For 3D printing and image generation, this field is not used.

The provided post-processing scripts can read these files and output a single greyscale image.

#### 3D Printing
To 3D print a terrain map, the greyscale image needs to be converted into an STL file. There are several free tools which can do this, such as [this one](http://clonerbox.com/image_3D_converter.php).

The STL file can then be 3D printed using your toolchain of choice.

## Example
[Workflow Example](.\Example\Example.md)

## Compilation / Dependencies
[Building from the source](.\Building.md)