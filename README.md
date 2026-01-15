#Basic Compression Tool v0.4

A simple command-line compression tool implementing Lempel-Ziv (LZ) and Run-Length Encoding (RLE) algorithms.
This project was created as a learning exercise while studying data compression and file handling in C++.

Features:

-Compress files
-Decompress previously compressed files
-Works with any file type (binary-safe)
-Command-line interface
-Basic compression/decompression statistics

Requirements
C++ compiler (GCC, Clang, MSVC, etc.)

Tested on Windows, should also work on Linux/macOS

##How to Compile:
Open a terminal in the project root folder and run:
g++ src/*.cpp -o compress

##How to Use:
###Command Format:
.\compress <algorithm> <mode> <input_file> <output_file>

##Parameters:
<algorithm>

lz – Lempel–Ziv compression (must be decompressed using lz)

rle – Run-Length Encoding (must be decompressed using rle)

<mode>

compress

decompress

<input_file>

Any file located in the same folder as the executable
Include the file extension

<output_file>

Any filename
Recommended extensions:
.lz for LZ-compressed files

.rle for RLE-compressed files

##Example:
.\compress lz compress test.txt test.lz

##Notes
Files must be decompressed using the same algorithm they were compressed with.
This project is intended for educational purposes and is not optimized for maximum compression efficiency.