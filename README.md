[example-img]: ./img/example.png

# Svg2Svs

![Example pyramid][example-img]

A small tool to convert `.svg` images (potentially any format supported by libvips) into Aperio's `.svs` format.

The `svg2svs` utility program does something similar to:

``` sh
vips im_vips2tiff checker.png output.tif:deflate,tile:32x32,pyramid
```
But libvips cannot generate specific scales of the pyramid, nor generate a mock `.svs` format
with the internal and non-standard metadata like __MPP__ or __AppMag__ .

## Compile and run

We depend on `libvips`, `libtiff` and on `pycairo` for the svg generation.
On Ubuntu/Debian, simply run:

``` sh
pip3 install --user "pycairo>=1.18.0" && sudo apt install libvips-dev libtiff-dev
```

on MacOs:

``` sh
pip3 install --user "pycairo>=1.18.0" && brew install libvips libtiff
```

Once the dependencies are installed, type `make`.
To generate a sample 10x10 with 10 subdivisions `checkerboard.svg`, run:

``` sh
python generate_svg_checkerboard.py  --divisions=10 --subdivisions=10 checkerboard.svg
```

To generate a 4 layers `checkerboard.svs` pyramidal tiff file with scalings 4,16 and 64, run:

``` sh
./svg2svs -l 4,16,48 checkerboard.svg checkerboard.svs
```

## Units

By default, the 10mmx10mm svg is rasterized into 38 pixels. This is because default dpi is 96 (96 / 25.4 = 3.78 pixels per mm).
To sample a theoretical base image at 1 MPP, we need 1000pixels per mm. This is achieved by 25400 dpi.

## Tiffinfo

It's often useful to debug svs files by reading its content metadata/structure.
This can be done using tiffinfo, for instance:

``` sh
$ tiffinfo example.svs

TIFF Directory at offset 0x225fa44e (576693326)
  Subfile Type: (0 = 0x0)
  Image Width: 74752 Image Length: 89856 Image Depth: 1
  Tile Width: 256 Tile Length: 256
  Bits/Sample: 8
  Compression Scheme: JPEG
  Photometric Interpretation: RGB color
  YCbCr Subsampling: 2, 2
  Samples/Pixel: 3
  Planar Configuration: single image plane
  ImageDescription: Aperio Image Library v11.1.9
74752x89856 (256x256) JPEG/RGB Q=70;Mirax Digital Slide|AppMag = 20|MPP = 0.23250
  JPEG Tables: (289 bytes)
TIFF Directory at offset 0x226cd23e (577557054)
  Subfile Type: (0 = 0x0)
  Image Width: 638 Image Length: 768 Image Depth: 1
  Bits/Sample: 8
  Compression Scheme: JPEG
  Photometric Interpretation: RGB color
  YCbCr Subsampling: 2, 2
  Samples/Pixel: 3
  Rows/Strip: 16
  Planar Configuration: single image plane
  ImageDescription: Aperio Image Library v11.1.9
74752x89856 -> 638x768 - ;Mirax Digital Slide|AppMag = 20|MPP = 0.23250
  JPEG Tables: (289 bytes)
TIFF Directory at offset 0x257dadc8 (628993480)
  Subfile Type: (0 = 0x0)
  Image Width: 18688 Image Length: 22464 Image Depth: 1
  Tile Width: 256 Tile Length: 256
  Bits/Sample: 8
  Compression Scheme: JPEG
  Photometric Interpretation: RGB color
  YCbCr Subsampling: 2, 2
  Samples/Pixel: 3
  Planar Configuration: single image plane
  ImageDescription: Aperio Image Library v11.1.9
74752x89856 (256x256) -> 18688x22464 JPEG/RGB Q=85
  JPEG Tables: (289 bytes)
TIFF Directory at offset 0x25bf8144 (633307460)
  Subfile Type: (0 = 0x0)
  Image Width: 4672 Image Length: 5616 Image Depth: 1
  Tile Width: 256 Tile Length: 256
  Bits/Sample: 8
  Compression Scheme: JPEG
  Photometric Interpretation: RGB color
  YCbCr Subsampling: 2, 2
  Samples/Pixel: 3
  Planar Configuration: single image plane
  ImageDescription: Aperio Image Library v11.1.9
74752x89856 (256x256) -> 4672x5616 JPEG/RGB Q=92
  JPEG Tables: (289 bytes)
TIFF Directory at offset 0x25d4aedc (634695388)
  Subfile Type: (0 = 0x0)
  Image Width: 2336 Image Length: 2808 Image Depth: 1
  Tile Width: 256 Tile Length: 256
  Bits/Sample: 8
  Compression Scheme: JPEG
  Photometric Interpretation: RGB color
  YCbCr Subsampling: 2, 2
  Samples/Pixel: 3
  Planar Configuration: single image plane
  ImageDescription: Aperio Image Library v11.1.9
74752x89856 (256x256) -> 2336x2808 JPEG/RGB Q=96
  JPEG Tables: (289 bytes)
TIFF Directory at offset 0x25dc70de (635203806)
  Subfile Type: reduced-resolution image (1 = 0x1)
  Image Width: 641 Image Length: 481
  Resolution: 96, 96 pixels/inch
  Bits/Sample: 8
  Compression Scheme: LZW
  Photometric Interpretation: RGB color
  Samples/Pixel: 3
  Rows/Strip: 6
  Planar Configuration: single image plane
  Predictor: none 1 (0x1)
TIFF Directory at offset 0x25d96cfc (635006204)
  Subfile Type: reduced-resolution image (9 = 0x9)
  Image Width: 515 Image Length: 1134 Image Depth: 1
  Bits/Sample: 8
  Compression Scheme: JPEG
  Photometric Interpretation: RGB color
  YCbCr Subsampling: 2, 2
  Samples/Pixel: 3
  Rows/Strip: 16
  Planar Configuration: single image plane
  ImageDescription: Aperio Image Library v11.1.9
macro 515x1134
  JPEG Tables: (289 bytes)
```

