#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>  // For sleep
#include <algorithm>

// Include your logic headers
#include "aes.h"
#include "AESModes.h" 

// --- COLORS & STYLING ---
// ANSI Escape Codes for text color
const std::string RESET   = "\033[0m";
const std::string RED     = "\033[31m";
const std::string GREEN   = "\033[32m";
const std::string YELLOW  = "\033[33m";
const std::string BLUE    = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN    = "\033[36m";
const std::string WHITE   = "\033[37m";
const std::string BOLD    = "\033[1m";

// --- UTILITY FUNCTIONS ---

void printHeader() {
    std::cout << CYAN << BOLD;
    // R"(...)" is a Raw String Literal. It ignores escape characters like backslashes.
    std::cout << R"(
   ================================================================
      _    _____ ____     _____                  _        
     / \  | ____/ ___|   / ____|_ __ _   _ _ __ | |_ ___  
    / _ \ |  _| \___ \  | |   | '__| | | | '_ \| __/ _ \ 
   / ___ \| |___ ___) | | |___| |  | |_| | |_) | || (_) |
  /_/   \_\_____|____/   \_____|_|   \__, | .__/ \__\___/ 
                                     |___/|_|             
            Student Independent Study 1: AES Implementation
   ================================================================
)" << "\n";
    std::cout << RESET;
}

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
    // Cross-platform clear
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Simulated progress bar for "Heavy" crypto operations
void showProgressBar(const std::string& task) {
    std::cout << YELLOW << task << "... " << RESET;
    std::cout << "[";
    for (int i = 0; i <= 20; ++i) {
        std::cout << "#";
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Fake delay for effect
    }
    std::cout << "] " << GREEN << "Done!" << RESET << "\n";
}

// --- MENU FUNCTIONS ---

int getIntInput(const std::string& prompt, int min, int max) {
    int choice;
    while (true) {
        std::cout << BOLD << prompt << RESET;
        if (std::cin >> choice && choice >= min && choice <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return choice;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << RED << "Invalid input. Please enter a number between " << min << " and " << max << ".\n" << RESET;
    }
}

std::string getStringInput(const std::string& prompt) {
    std::string line;
    std::cout << BOLD << prompt << RESET;
    std::getline(std::cin, line);
    return line;
}

void waitForKey() {
    std::cout << "\n" << MAGENTA << "Press Enter to continue..." << RESET;
    std::cin.get();
}

// --- MAIN APPLICATION LOGIC ---

int main() {
    while (true) {
        clearScreen();
        printHeader();
        
        std::cout << "   " << BOLD << "MAIN MENU" << RESET << "\n";
        std::cout << "   " << CYAN << "[1]" << RESET << " Encrypt Data\n";
        std::cout << "   " << CYAN << "[2]" << RESET << " Decrypt Data\n";
        std::cout << "   " << YELLOW << "[3]" << RESET << " Bonus: ECB Image Weakness Demo\n";
        std::cout << "   " << RED << "[4]" << RESET << " Exit\n\n";
        
        int action = getIntInput("   Select Action: ", 1, 4);

        if (action == 4) {
            std::cout << "\n   " << GREEN << "Exiting application. Goodbye!" << RESET << "\n";
            break;
        }

        try {
            // === SPECIAL MODE: ECB IMAGE DEMO ===
            if (action == 3) {
                std::cout << "\n   " << YELLOW << "--- ECB WEAKNESS DEMO (BMP ONLY) ---" << RESET << "\n";
                std::string inFile = getStringInput("   Enter input BMP filename (e.g., tux.bmp): ");
                std::string outFile = getStringInput("   Enter output filename (e.g., tux_ecb.bmp): ");
                
                showProgressBar("Reading File");
                std::vector<uint8_t> fileData = readFile(inFile);
                
                if (fileData.size() < 54) throw std::runtime_error("File too small to be a BMP");

                std::vector<uint8_t> header(fileData.begin(), fileData.begin() + 54);
                std::vector<uint8_t> pixelData(fileData.begin() + 54, fileData.end());

                std::vector<uint8_t> key(16, 0x55); 
                AESModes cipher(key.data(), key.size());

                showProgressBar("Encrypting Pixels (ECB)");
                std::vector<uint8_t> encryptedPixels = cipher.encryptECB(pixelData);

                std::vector<uint8_t> finalOutput = header;
                finalOutput.insert(finalOutput.end(), encryptedPixels.begin(), encryptedPixels.end());

                writeFile(outFile, finalOutput);
                std::cout << "\n   " << GREEN << "[SUCCESS]" << RESET << " Image encrypted with header preserved.\n";
                std::cout << "   Open " << BOLD << outFile << RESET << " to see the ECB patterns!\n";
                
                waitForKey();
                continue;
            }

            // === STANDARD MODES (1, 2) ===

            // 1. SELECT KEY SIZE
            std::cout << "\n   " << BOLD << "--- CONFIGURATION ---" << RESET << "\n";
            std::cout << "   1. AES-128\n   2. AES-192\n   3. AES-256\n";
            int keyChoice = getIntInput("   Select Key Size: ", 1, 3);
            int keySize = (keyChoice == 1) ? 16 : (keyChoice == 2) ? 24 : 32;

            // 2. GENERATE OR INPUT KEY
            std::cout << "\n   1. Generate Random Key\n   2. Enter Hex Manually\n";
            int keySource = getIntInput("   Key Source: ", 1, 2);

            std::vector<uint8_t> key(keySize);
            if (keySource == 1) {
                CustomRNG rng;
                rng.getBytes(key.data(), keySize);
                std::cout << "   " << GREEN << "Generated Key: " << RESET; printHex(key);
            } else {
                std::string hexKey;
                while (true) {
                    hexKey = getStringInput("   Enter Key (Hex): ");
                    if (hexKey.length() == keySize * 2) break;
                    std::cout << "   " << RED << "Invalid length! Expected " << keySize * 2 << " hex characters." << RESET << "\n";
                }
                key = hexToBytes(hexKey);
            }

            AESModes cipher(key.data(), key.size());

            // 3. SELECT MODE
            std::cout << "\n   " << BOLD << "--- MODE SELECTION ---" << RESET << "\n";
            std::cout << "   1. ECB (Electronic Codebook)\n";
            std::cout << "   2. CBC (Cipher Block Chaining)\n";
            std::cout << "   3. CTR (Counter Mode)\n";
            std::cout << "   4. GCM (Galois/Counter Mode)\n";
            int modeChoice = getIntInput("   Select Mode: ", 1, 4);

            // 4. SELECT INPUT SOURCE
            std::cout << "\n   " << BOLD << "--- DATA INPUT ---" << RESET << "\n";
            std::cout << "   1. Text String (Console)\n   2. File (Binary)\n";
            int inputSource = getIntInput("   Input Source: ", 1, 2);
            
            std::vector<uint8_t> inputData;
            std::string outFilename;

            if (inputSource == 1) {
                if (action == 1) { 
                    std::string text = getStringInput("   Enter Plaintext: ");
                    inputData.assign(text.begin(), text.end());
                } else { 
                    std::string hex = getStringInput("   Enter Ciphertext (Hex): ");
                    inputData = hexToBytes(hex);
                }
            } else {
                std::string filename = getStringInput("   Enter Input Filename: ");
                showProgressBar("Reading File");
                inputData = readFile(filename);
                outFilename = getStringInput("   Enter Output Filename: ");
            }

            // 5. PERFORM OPERATION
            std::vector<uint8_t> outputData;
            std::vector<uint8_t> aad;

            // GCM AAD Handling
            if (modeChoice == 4) {
                 std::string prompt = (action == 1) ? "   Enter AAD (Optional): " : "   Enter AAD used during encryption: ";
                 std::string aadStr = getStringInput(prompt);
                 aad.assign(aadStr.begin(), aadStr.end());
            }

            std::cout << "\n";
            showProgressBar("Processing Crypto Engine");

            auto start = std::chrono::high_resolution_clock::now();

            if (action == 1) { // ENCRYPTION
                if (modeChoice == 4) outputData = cipher.encryptGCM(inputData, aad);
                else if (modeChoice == 1) outputData = cipher.encryptECB(inputData);
                else if (modeChoice == 2) outputData = cipher.encryptCBC(inputData);
                else if (modeChoice == 3) outputData = cipher.encryptCTR(inputData);
            } else { // DECRYPTION
                if (modeChoice == 4) outputData = cipher.decryptGCM(inputData, aad);
                else if (modeChoice == 1) outputData = cipher.decryptECB(inputData);
                else if (modeChoice == 2) outputData = cipher.decryptCBC(inputData);
                else if (modeChoice == 3) outputData = cipher.decryptCTR(inputData);
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;

            // 6. OUTPUT RESULTS
            std::cout << "\n   " << GREEN << "--- OPERATION SUCCESSFUL ---" << RESET << "\n";
            std::cout << "   Time elapsed: " << std::fixed << std::setprecision(4) << elapsed.count() << " ms\n";
            std::cout << "   Throughput:   " << (inputData.size() / 1024.0 / 1024.0) / (elapsed.count() / 1000.0) << " MB/s\n";

            if (inputSource == 1) {
                if (action == 1) {
                    std::cout << "   Ciphertext (Hex): "; printHex(outputData);
                } else {
                    std::string recovered(outputData.begin(), outputData.end());
                    std::cout << "   Recovered Plaintext: " << recovered << "\n";
                }
            } else {
                writeFile(outFilename, outputData);
                std::cout << "   Output saved to " << BOLD << outFilename << RESET << "\n";
            }

        } catch (const std::exception& e) {
            std::cout << "\n   " << RED << "[ERROR]: " << e.what() << RESET << "\n";
        }

        waitForKey();
    }

    return 0;
}