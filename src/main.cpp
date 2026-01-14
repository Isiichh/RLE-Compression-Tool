#include <iostream>
#include <chrono>
#include <filesystem>
#include "rle.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage:\n";
        std::cout << "  rle compress <input> <output>\n";
        std::cout << "  rle decompress <input> <output>\n";
        return 1;
    }

    std::string mode = argv[1];
    std::string input = argv[2];
    std::string output = argv[3];

    bool success = false;

    uintmax_t inputSize = 0;
    uintmax_t outputSize = 0;

    try {
        inputSize = std::filesystem::file_size(input);
    } catch (...) {
        std::cerr << "Failed to read input file size\n";
    }

    auto start = std::chrono::high_resolution_clock::now();

    if (mode == "compress") {
        std::cout << "Compressing: " << input << " -> " << output << "\n";
        success = compressRLE(input, output);
    } else if (mode == "decompress") {
        std::cout << "Decompressing: " << input << " -> " << output << "\n";
        success = decompressRLE(input, output);
    } else {
        std::cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    if (!success) {
        std::cerr << "Operation failed!\n";
        return 1;
    }

    try {
        outputSize = std::filesystem::file_size(output);
    } catch (...) {
        std::cerr << "Failed to read output file size\n";
    }

    std::cout << "\n--- Stats ---\n";
    std::cout << "Input size:  " << inputSize << " bytes\n";
    std::cout << "Output size: " << outputSize << " bytes\n";
    std::cout << "Time:        " << elapsed.count() << " seconds\n";

    if (mode == "compress" && inputSize > 0) {
        double ratio = 100.0 * (1.0 - (double)outputSize / inputSize);
        std::cout << "Compression ratio: " << ratio << "%\n";
    }

    std::cout << "Operation completed successfully.\n";
    return 0;
}
