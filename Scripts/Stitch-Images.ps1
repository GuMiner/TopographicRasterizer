<#
.SYNOPSIS
Stitches a directory of images together.

.PARAMETER SourceFolder
The folder with images (in their respective directories) to stitch

.PARAMETER DestinationImage
The image that will be written with all images stitched together.

.EXAMPLES
.\Stitch-Images.ps1 -SourceFolder "C:\path\to\rasters" -DestinationImage "C:\path\to\output.png" -Verbose
#>
param(
    [Parameter(Mandatory=$true)]
    [string]
    $SourceFolder,

    [Parameter(Mandatory=$true)]
    [string]
    $DestinationImage)

Add-Type -AssemblyName System.Drawing

$tileCount = [IO.Directory]::GetDirectories($SourceFolder).Count

# Scan the first image to determine the image size
$testImage = New-Object System.Drawing.Bitmap "$SourceFolder\0\0.png"
$imageSize = $testImage.Width
$testImage.Dispose()

# Draw each image into a mega-image and then save it.
$totalSize = $tileCount*$imageSize
$newImage = New-Object 'System.Drawing.Bitmap' -ArgumentList $totalSize,$totalSize
$graphics = [System.Drawing.Graphics]::FromImage($newImage)

$x = 0
foreach ($subdirectory in [IO.Directory]::GetDirectories($SourceFolder))
{
    $y = 0
    foreach ($file in [IO.Directory]::GetFiles($subdirectory))
    {
        Write-Output "Tiling $x, $y into the image..."
        $image = New-Object 'System.Drawing.Bitmap' -ArgumentList $file
        
        $graphics.DrawImage($image, $y*$imageSize, $x*$imageSize, $imageSize, $imageSize)
        ++$y
    }

    ++$x
}

$graphics.Dispose()

Write-Output "Saving the output image..."
$newImage.Save($DestinationImage)
Write-Output "Done."