# Exif-Tag-Viewer
Reads and displays metadata about the camera and exposure via EXIF tags that are often embedded in photos taken on digital cameras.

## accepted image formats
JPEG

## compilation
gcc -o exifviewer exifviewer.c

## command line arguments
./exifviewer image.jpg

## NOTE
Does not support bigendianess
It is possible no tag exists
