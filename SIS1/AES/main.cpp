#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

// Include your logic headers
#include "aes.h"
#include "AESModes.h" 

// --- UTILITY FUNCTIONS ---

// Display vector as Hex String
void printHex(const std::vector<uint8_t>& data) {
    for (uint8_t b : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    std::cout << std::dec << "\n";
}

// Convert Hex String to Vector
std::vector<uint8_t> hexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

// Read binary file
std::vector<uint8_t> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Could not open file.");
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
}

// Write binary file
void writeFile(const std::string& filename, const std::vector<uint8_t>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Could not write to file.");
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

// Clear console (optional, cross-platform friendly)
void clearScreen() {
    std::cout << "\n\n------------------------------------------------\n";
}

// --- MENU FUNCTIONS ---

int getIntInput(const std::string& prompt, int min, int max) {
    int choice;
    while (true) {
        std::cout << prompt;
        if (std::cin >> choice && choice >= min && choice <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer
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
        int action = getIntInput("Select Action (1-3): ", 1, 3);

        if (action == 3) break;

        try {
            // 1. SELECT KEY SIZE [Source: 115]
            int keyChoice = getIntInput("\nSelect Key Size:\n1. 128-bit\n2. 192-bit\n3. 256-bit\nChoice: ", 1, 3);
            int keySize = (keyChoice == 1) ? 16 : (keyChoice == 2) ? 24 : 32;

            // 2. GENERATE OR INPUT KEY [Source: 117]
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

            // Initialize Cipher Engine
            AESModes cipher(key.data(), key.size());

            // 3. SELECT MODE [Source: 116]
            int modeChoice = getIntInput("\nSelect Mode:\n1. ECB\n2. CBC\n3. CTR\n4. GCM\nChoice: ", 1, 4);

            // 4. SELECT INPUT SOURCE [Source: 118]
            int inputSource = getIntInput("\nInput Source:\n1. Text String\n2. File\nChoice: ", 1, 2);
            
            std::vector<uint8_t> inputData;
            std::string outFilename;

            if (inputSource == 1) {
                if (action == 1) { // Encrypt: Text -> Bytes
                    std::string text = getStringInput("Enter Plaintext: ");
                    inputData.assign(text.begin(), text.end());
                } else { // Decrypt: Hex -> Bytes
                    std::string hex = getStringInput("Enter Ciphertext (Hex): ");
                    inputData = hexToBytes(hex);
                }
            } else {
                std::string filename = getStringInput("Enter Input Filename: ");
                inputData = readFile(filename);
                outFilename = getStringInput("Enter Output Filename: ");
            }

            // 5. PERFORM OPERATION [Source: 119]
            std::vector<uint8_t> outputData;
            std::vector<uint8_t> aad; // Only for GCM

            // Start Timing [Source: 123]
            auto start = std::chrono::high_resolution_clock::now();

            if (action == 1) { // ENCRYPTION
                if (modeChoice == 4) { // GCM requires AAD
                    std::string aadStr = getStringInput("Enter AAD (Optional, press enter for none): ");
                    aad.assign(aadStr.begin(), aadStr.end());
                    outputData = cipher.encryptGCM(inputData, aad);
                }
                else if (modeChoice == 1) outputData = cipher.encryptECB(inputData);
                else if (modeChoice == 2) outputData = cipher.encryptCBC(inputData);
                else if (modeChoice == 3) outputData = cipher.encryptCTR(inputData);

            } else { // DECRYPTION
                if (modeChoice == 4) { // GCM
                    std::string aadStr = getStringInput("Enter AAD used during encryption: ");
                    aad.assign(aadStr.begin(), aadStr.end());
                    outputData = cipher.decryptGCM(inputData, aad);
                }
                else if (modeChoice == 1) outputData = cipher.decryptECB(inputData);
                else if (modeChoice == 2) outputData = cipher.decryptCBC(inputData);
                else if (modeChoice == 3) outputData = cipher.decryptCTR(inputData);
            }

            // End Timing
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;

            // 6. OUTPUT RESULTS [Source: 120]
            std::cout << "\n--- OPERATION SUCCESSFUL ---\n";
            std::cout << "Time elapsed: " << elapsed.count() << " ms\n";

            if (inputSource == 1) {
                // Text Output
                if (action == 1) {
                    std::cout << "Ciphertext (Hex): "; 
                    printHex(outputData);
                } else {
                    std::string recovered(outputData.begin(), outputData.end());
                    std::cout << "Recovered Plaintext: " << recovered << "\n";
                }
            } else {
                // File Output
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