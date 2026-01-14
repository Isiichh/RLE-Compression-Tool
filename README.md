RLE Compression Tool v0.2

A simple Run-Lenght Encoding compressor and decompressor in c++ made as a learning project

Features:
-Compress Files
-Decompress previously compressed files
-Works with any file type (binary safe)
-Command-line interface

Requirements:
C++ compiler
Tested on Windows, should work for Linux/macOS

How to compile:
Open a terminal in the project folder: 

```g++ src/*.cpp -o rle```


How to use:    
    Compression:
        ```.\rle compress input.txt output.rle```
    Decompression:
        ```.\rle decompress input.rle output.txt```