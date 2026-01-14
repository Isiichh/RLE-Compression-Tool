#include "rle.h"
#include <fstream>
#include <iostream>
#include <vector>

static constexpr size_t BUFFER_SIZE = 64 * 1024; // 64 KB

bool compressRLE(const std::string& input, const std::string& output) {
    std::ifstream inFile(input, std::ios::binary);
    if (!inFile) {
        std::cerr << "Cannot open input file\n";
        return false;
    }

    std::ofstream outFile(output, std::ios::binary);
    if (!outFile) {
        std::cerr << "Cannot open output file\n";
        return false;
    }

    char inBuffer[BUFFER_SIZE];
    std::vector<char> outBuffer;
    outBuffer.reserve(BUFFER_SIZE);

    char lastChar = 0;
    unsigned char count = 0;
    bool first = true;

    while (inFile) {
        inFile.read(inBuffer, BUFFER_SIZE);
        std::streamsize bytesRead = inFile.gcount();

        for (std::streamsize i = 0; i < bytesRead; ++i) {
            char current = inBuffer[i];

            if (first) {
                lastChar = current;
                count = 1;
                first = false;
            } else if (current == lastChar && count < 255) {
                ++count;
            } else {
                outBuffer.push_back(static_cast<char>(count));
                outBuffer.push_back(lastChar);

                lastChar = current;
                count = 1;
            }

            if (outBuffer.size() >= BUFFER_SIZE) {
                outFile.write(outBuffer.data(), outBuffer.size());
                outBuffer.clear();
            }
        }
    }

    // Flush last run
    if (!first) {
        outBuffer.push_back(static_cast<char>(count));
        outBuffer.push_back(lastChar);
    }

    // Flush remaining output
    if (!outBuffer.empty()) {
        outFile.write(outBuffer.data(), outBuffer.size());
    }

    return true;
}

bool decompressRLE(const std::string& input, const std::string& output) {
    std::ifstream inFile(input, std::ios::binary);
    if (!inFile) {
        std::cerr << "Cannot open input file\n";
        return false;
    }

    std::ofstream outFile(output, std::ios::binary);
    if (!outFile) {
        std::cerr << "Cannot open output file\n";
        return false;
    }

    char inBuffer[BUFFER_SIZE];
    std::vector<char> outBuffer;
    outBuffer.reserve(BUFFER_SIZE);

    while (inFile) {
        inFile.read(inBuffer, BUFFER_SIZE);
        std::streamsize bytesRead = inFile.gcount();

        for (std::streamsize i = 0; i + 1 < bytesRead; i += 2) {
            unsigned char count = static_cast<unsigned char>(inBuffer[i]);
            char value = inBuffer[i + 1];

            for (unsigned char j = 0; j < count; ++j) {
                outBuffer.push_back(value);

                if (outBuffer.size() >= BUFFER_SIZE) {
                    outFile.write(outBuffer.data(), outBuffer.size());
                    outBuffer.clear();
                }
            }
        }
    }

    if (!outBuffer.empty()) {
        outFile.write(outBuffer.data(), outBuffer.size());
    }

    return true;
}
