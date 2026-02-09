#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

#include "aes.h"
#include "AESModes.h" 

// --- UTILITY FUNCTIONS ---

void printHex(const std::vector<uint8_t>& data) {
    for (uint8_t b : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    std::cout << std::dec << "\n";
}

std::vector<uint8_t> hexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

std::vector<uint8_t> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Could not open file.");
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
}

void writeFile(const std::string& filename, const std::vector<uint8_t>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Could not write to file.");
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void clearScreen() {
    std::cout << "\n\n------------------------------------------------\n";
}

// --- MENU FUNCTIONS ---

int getIntInput(const std::string& prompt, int min, int max) {
    int choice;
    while (true) {
        std::cout << prompt;
        if (std::cin >> choice && choice >= min && choice <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return choice;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please try again.\n";
    }
}

std::string getStringInput(const std::string& prompt) {
    std::string line;
    std::cout << prompt;
    std::getline(std::cin, line);
    return line;
}

// --- MAIN APPLICATION LOGIC ---

int main() {
    std::cout << "================================================\n";
    std::cout << "   SIS1 AES IMPLEMENTATION - CLI INTERFACE\n";
    std::cout << "================================================\n";

    while (true) {
        clearScreen();
        std::cout << "MAIN MENU:\n";
        std::cout << "1. Encrypt Data\n";
        std::cout << "2. Decrypt Data\n";
        std::cout << "3. Exit\n";
        std::cout << "4. [BONUS] ECB Image Weakness Demo\n"; // New Feature
        int action = getIntInput("Select Action (1-4): ", 1, 4);

        if (action == 3) break;

        try {
            // === SPECIAL MODE: ECB IMAGE DEMO ===
            if (action == 4) {
                std::cout << "\n--- ECB WEAKNESS DEMO (BMP ONLY) ---\n";
                std::string inFile = getStringInput("Enter input BMP filename (e.g., tux.bmp): ");
                std::string outFile = getStringInput("Enter output filename (e.g., tux_ecb.bmp): ");
                
                std::vector<uint8_t> fileData = readFile(inFile);
                
                if (fileData.size() < 54) throw std::runtime_error("File too small to be a BMP");

                // 1. Separate Header (54 bytes) and Pixel Data
                std::vector<uint8_t> header(fileData.begin(), fileData.begin() + 54);
                std::vector<uint8_t> pixelData(fileData.begin() + 54, fileData.end());

                // 2. Setup Cipher (Fixed Key for demo)
                std::vector<uint8_t> key(16, 0x55); // Dummy key
                AESModes cipher(key.data(), key.size());

                // 3. Encrypt ONLY the pixel data using ECB
                // Note: ECB pads data, so output might be slightly larger. 
                // For a perfect visual demo, we usually truncate extra padding, 
                // but standard encryption is fine for visualization.
                std::vector<uint8_t> encryptedPixels = cipher.encryptECB(pixelData);

                // 4. Reconstruct: Header + Encrypted Pixels
                std::vector<uint8_t> finalOutput = header;
                finalOutput.insert(finalOutput.end(), encryptedPixels.begin(), encryptedPixels.end());

                writeFile(outFile, finalOutput);
                std::cout << "\n[SUCCESS] Image encrypted with header preserved.\n";
                std::cout << "Open " << outFile << " to see the ECB patterns!\n";
                
                std::cout << "\nPress Enter to continue...";
                std::cin.get();
                continue;
            }

            // === STANDARD MODES (1, 2) ===

            // 1. SELECT KEY SIZE
            int keyChoice = getIntInput("\nSelect Key Size:\n1. 128-bit\n2. 192-bit\n3. 256-bit\nChoice: ", 1, 3);
            int keySize = (keyChoice == 1) ? 16 : (keyChoice == 2) ? 24 : 32;

            // 2. GENERATE OR INPUT KEY
            std::vector<uint8_t> key(keySize);
            int keySource = getIntInput("\nKey Source:\n1. Generate Random Key\n2. Enter Hex Manually\nChoice: ", 1, 2);

            if (keySource == 1) {
                CustomRNG rng;
                rng.getBytes(key.data(), keySize);
                std::cout << "Generated Key: "; printHex(key);
            } else {
                std::string hexKey;
                while (true) {
                    hexKey = getStringInput("Enter Key (Hex): ");
                    if (hexKey.length() == keySize * 2) break;
                    std::cout << "Invalid length! Expected " << keySize * 2 << " hex characters.\n";
                }
                key = hexToBytes(hexKey);
            }

            AESModes cipher(key.data(), key.size());

            // 3. SELECT MODE
            int modeChoice = getIntInput("\nSelect Mode:\n1. ECB\n2. CBC\n3. CTR\n4. GCM\nChoice: ", 1, 4);

            // 4. SELECT INPUT SOURCE
            int inputSource = getIntInput("\nInput Source:\n1. Text String\n2. File\nChoice: ", 1, 2);
            
            std::vector<uint8_t> inputData;
            std::string outFilename;

            if (inputSource == 1) {
                if (action == 1) { 
                    std::string text = getStringInput("Enter Plaintext: ");
                    inputData.assign(text.begin(), text.end());
                } else { 
                    std::string hex = getStringInput("Enter Ciphertext (Hex): ");
                    inputData = hexToBytes(hex);
                }
            } else {
                std::string filename = getStringInput("Enter Input Filename: ");
                inputData = readFile(filename);
                outFilename = getStringInput("Enter Output Filename: ");
            }

            // 5. PERFORM OPERATION
            std::vector<uint8_t> outputData;
            std::vector<uint8_t> aad;

            auto start = std::chrono::high_resolution_clock::now();

            if (action == 1) { // ENCRYPTION
                if (modeChoice == 4) {
                    std::string aadStr = getStringInput("Enter AAD (Optional): ");
                    aad.assign(aadStr.begin(), aadStr.end());
                    outputData = cipher.encryptGCM(inputData, aad);
                }
                else if (modeChoice == 1) outputData = cipher.encryptECB(inputData);
                else if (modeChoice == 2) outputData = cipher.encryptCBC(inputData);
                else if (modeChoice == 3) outputData = cipher.encryptCTR(inputData);
            } else { // DECRYPTION
                if (modeChoice == 4) {
                    std::string aadStr = getStringInput("Enter AAD used during encryption: ");
                    aad.assign(aadStr.begin(), aadStr.end());
                    outputData = cipher.decryptGCM(inputData, aad);
                }
                else if (modeChoice == 1) outputData = cipher.decryptECB(inputData);
                else if (modeChoice == 2) outputData = cipher.decryptCBC(inputData);
                else if (modeChoice == 3) outputData = cipher.decryptCTR(inputData);
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;

            // 6. OUTPUT RESULTS
            std::cout << "\n--- OPERATION SUCCESSFUL ---\n";
            std::cout << "Time elapsed: " << elapsed.count() << " ms\n";

            if (inputSource == 1) {
                if (action == 1) {
                    std::cout << "Ciphertext (Hex): "; printHex(outputData);
                } else {
                    std::string recovered(outputData.begin(), outputData.end());
                    std::cout << "Recovered Plaintext: " << recovered << "\n";
                }
            } else {
                writeFile(outFilename, outputData);
                std::cout << "Output saved to " << outFilename << "\n";
            }

        } catch (const std::exception& e) {
            std::cout << "\n[ERROR]: " << e.what() << "\n";
        }

        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    return 0;
}