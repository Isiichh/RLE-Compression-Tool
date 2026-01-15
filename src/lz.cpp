#include "lz.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>

static constexpr size_t WINDOW_SIZE = 32 * 1024;
static constexpr size_t MIN_MATCH = 3;
static constexpr size_t MAX_MATCH = 255;
static constexpr size_t MAX_SEARCH = 4096;  // Search only 4KB back (SAFE)

void showProgress(size_t processed, size_t total) {
    static int lastPercent = -1;
    
    if (total == 0) return;
    
    int percent = static_cast<int>((processed * 100) / total);
    
    if (percent != lastPercent) {
        lastPercent = percent;
        
        const int barWidth = 20;
        int filled = (percent * barWidth) / 100;
        
        std::cout << "\r[";
        for (int i = 0; i < barWidth; i++) {
            std::cout << (i < filled ? '#' : '-');
        }
        std::cout << "] " << percent << "%" << std::flush;
    }
}

bool compressLZ(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream in(inputFile, std::ios::binary);
    if (!in) { std::cerr << "Cannot open input file\n"; return false; }

    std::ofstream out(outputFile, std::ios::binary);
    if (!out) { std::cerr << "Cannot open output file\n"; return false; }

    in.seekg(0, std::ios::end);
    size_t fileSize = in.tellg();
    in.seekg(0, std::ios::beg);
    
    std::cout << "Compressing " << fileSize << " bytes...\n";
    
    std::vector<uint8_t> input(fileSize);
    in.read(reinterpret_cast<char*>(input.data()), fileSize);
    
    std::vector<uint8_t> window(WINDOW_SIZE, 0);
    size_t windowPos = 0;
    size_t windowUsed = 0;
    
    std::vector<uint8_t> output;
    output.reserve(fileSize / 2);
    
    size_t i = 0;
    size_t lastUpdate = 0;
    const size_t UPDATE_INTERVAL = fileSize / 50;  // Update only 50 times
    
    while (i < fileSize) {
        uint8_t flagByte = 0;
        std::vector<uint8_t> chunkData;
        
        for (int bit = 0; bit < 8 && i < fileSize; bit++) {
            size_t bestLen = 0;
            size_t bestOffset = 0;
            
            // OPTIMIZED SIMPLE SEARCH - LIMIT SEARCH DISTANCE
            if (windowUsed >= MIN_MATCH && i + MIN_MATCH <= fileSize) {
                // Only search the most recent MAX_SEARCH bytes
                size_t searchLimit = std::min(windowUsed, MAX_SEARCH);
                
                // Start from 1 byte back, go up to searchLimit
                for (size_t offset = 1; offset <= searchLimit; offset++) {
                    size_t srcPos = (windowPos + WINDOW_SIZE - offset) % WINDOW_SIZE;
                    
                    // Quick early rejection: check first byte
                    if (window[srcPos] != input[i]) continue;
                    
                    // Check minimum match length
                    size_t len = 1;
                    while (len < MIN_MATCH && 
                           i + len < fileSize &&
                           window[(srcPos + len) % WINDOW_SIZE] == input[i + len]) {
                        len++;
                    }
                    
                    if (len < MIN_MATCH) continue;
                    
                    // Extend to full match
                    while (len < MAX_MATCH && 
                           i + len < fileSize &&
                           len < offset &&  // Can't match more than offset
                           window[(srcPos + len) % WINDOW_SIZE] == input[i + len]) {
                        len++;
                    }
                    
                    if (len > bestLen) {
                        bestLen = len;
                        bestOffset = offset;
                        // Good enough match? Stop searching this position
                        if (len >= 16) break;
                    }
                }
            }
            
            if (bestLen >= MIN_MATCH) {
                flagByte |= (1 << bit);
                
                chunkData.push_back((bestOffset - 1) & 0xFF);
                chunkData.push_back(((bestOffset - 1) >> 8) & 0xFF);
                chunkData.push_back(bestLen);
                
                // Update window
                for (size_t j = 0; j < bestLen; j++) {
                    window[windowPos] = input[i + j];
                    windowPos = (windowPos + 1) % WINDOW_SIZE;
                    if (windowUsed < WINDOW_SIZE) windowUsed++;
                }
                
                i += bestLen;
            } else {
                chunkData.push_back(input[i]);
                
                window[windowPos] = input[i];
                windowPos = (windowPos + 1) % WINDOW_SIZE;
                if (windowUsed < WINDOW_SIZE) windowUsed++;
                
                i++;
            }
        }
        
        output.push_back(flagByte);
        output.insert(output.end(), chunkData.begin(), chunkData.end());
        
        // Less frequent progress updates
        if (i - lastUpdate >= UPDATE_INTERVAL) {
            showProgress(i, fileSize);
            lastUpdate = i;
        }
    }
    
    showProgress(fileSize, fileSize);
    
    out.write(reinterpret_cast<char*>(output.data()), output.size());
    
    std::cout << "\nCompressed: " << fileSize << " -> " << output.size() 
              << " bytes (" << (output.size() * 100.0 / fileSize) << "%)\n";
    
    return true;
}

bool decompressLZ(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream in(inputFile, std::ios::binary);
    if (!in) { std::cerr << "Cannot open input file\n"; return false; }

    std::ofstream out(outputFile, std::ios::binary);
    if (!out) { std::cerr << "Cannot open output file\n"; return false; }

    in.seekg(0, std::ios::end);
    size_t compressedSize = in.tellg();
    in.seekg(0, std::ios::beg);
    
    std::cout << "Decompressing " << compressedSize << " bytes...\n";
    
    std::vector<uint8_t> compressed(compressedSize);
    in.read(reinterpret_cast<char*>(compressed.data()), compressedSize);
    
    std::vector<uint8_t> window(WINDOW_SIZE, 0);
    size_t windowPos = 0;
    size_t windowUsed = 0;
    
    std::vector<uint8_t> output;
    output.reserve(compressedSize * 3);
    
    size_t i = 0;
    size_t lastUpdate = 0;
    const size_t UPDATE_INTERVAL = compressedSize / 50;
    
    while (i < compressedSize) {
        uint8_t flagByte = compressed[i++];
        
        for (int bit = 0; bit < 8 && i < compressedSize; bit++) {
            if (flagByte & (1 << bit)) {
                if (i + 3 > compressedSize) break;
                
                uint16_t offset = compressed[i] | (compressed[i+1] << 8);
                offset += 1;
                uint8_t length = compressed[i+2];
                i += 3;
                
                if (offset == 0 || offset > WINDOW_SIZE || length < MIN_MATCH) {
                    std::cerr << "Invalid match data\n";
                    return false;
                }
                
                size_t srcPos = (windowPos + WINDOW_SIZE - offset) % WINDOW_SIZE;
                for (size_t j = 0; j < length; j++) {
                    uint8_t byte = window[(srcPos + j) % WINDOW_SIZE];
                    output.push_back(byte);
                    
                    window[windowPos] = byte;
                    windowPos = (windowPos + 1) % WINDOW_SIZE;
                    if (windowUsed < WINDOW_SIZE) windowUsed++;
                }
            } else {
                if (i >= compressedSize) break;
                
                uint8_t literal = compressed[i++];
                output.push_back(literal);
                
                window[windowPos] = literal;
                windowPos = (windowPos + 1) % WINDOW_SIZE;
                if (windowUsed < WINDOW_SIZE) windowUsed++;
            }
        }
        
        if (i - lastUpdate >= UPDATE_INTERVAL) {
            showProgress(i, compressedSize);
            lastUpdate = i;
        }
    }
    
    showProgress(compressedSize, compressedSize);
    
    out.write(reinterpret_cast<char*>(output.data()), output.size());
    
    std::cout << "\nDecompressed: " << compressedSize << " -> " << output.size() 
              << " bytes\n";
    
    return true;
}