#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>  // (optional) for sleep / demo effects
#include <algorithm>

// AES core and modes (ECB/CBC/CTR/GCM)
#include "aes.h"
#include "AESModes.h" 

// ------------------- COLORS & STYLING -------------------
// ANSI escape codes to make the console UI nicer
const std::string RESET   = "\033[0m";
const std::string RED     = "\033[31m";
const std::string GREEN   = "\033[32m";
const std::string YELLOW  = "\033[33m";
const std::string BLUE    = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN    = "\033[36m";
const std::string WHITE   = "\033[37m";
const std::string BOLD    = "\033[1m";

// ------------------- UTILITY FUNCTIONS -------------------

// Prints a big banner header (ASCII art) for the program
void printHeader() {
    std::cout << CYAN << BOLD;

    // Raw string literal R"( ... )" prints text exactly as written (no escaping needed)
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

// Print a byte vector as a hex string (used for keys and ciphertext output)
void printHex(const std::vector<uint8_t>& data) {
    for (uint8_t b : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    std::cout << std::dec << "\n";
}

// Convert a hex string like "0a1b..." into raw bytes
std::vector<uint8_t> hexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

// Read an entire file as binary bytes
std::vector<uint8_t> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Could not open file.");

    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
}

// Write binary bytes to a file
void writeFile(const std::string& filename, const std::vector<uint8_t>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Could not write to file.");

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

// Clear the console screen (Windows/Linux)
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Simple progress bar for UI (no real delay unless you enable sleep)
void showProgressBar(const std::string& task) {
    std::cout << YELLOW << task << "... " << RESET;
    std::cout << "[";
    for (int i = 0; i <= 20; ++i) {
        std::cout << "#";
        std::cout.flush();
        // std::this_thread::sleep_for(std::chrono::milliseconds(20)); // optional demo delay
    }
    std::cout << "] " << GREEN << "Done!" << RESET << "\n";
}

// ------------------- MENU INPUT HELPERS -------------------

// Read an integer from user with range validation
int getIntInput(const std::string& prompt, int min, int max) {
    int choice;
    while (true) {
        std::cout << BOLD << prompt << RESET;

        // Valid integer and in range
        if (std::cin >> choice && choice >= min && choice <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return choice;
        }

        // Invalid input -> clean stream and ask again
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << RED << "Invalid input. Please enter a number between "
                  << min << " and " << max << ".\n" << RESET;
    }
}

// Read a full line string from user
std::string getStringInput(const std::string& prompt) {
    std::string line;
    std::cout << BOLD << prompt << RESET;
    std::getline(std::cin, line);
    return line;
}

// Pause until user presses Enter
void waitForKey() {
    std::cout << "\n" << MAGENTA << "Press Enter to continue..." << RESET;
    std::cin.get();
}

// ------------------- MAIN APPLICATION -------------------

int main() {
    while (true) {
        clearScreen();
        printHeader();

        // Main menu UI
        std::cout << "   " << BOLD << "MAIN MENU" << RESET << "\n";
        std::cout << "   " << CYAN   << "[1]" << RESET << " Encrypt Data\n";
        std::cout << "   " << CYAN   << "[2]" << RESET << " Decrypt Data\n";
        std::cout << "   " << YELLOW << "[3]" << RESET << " Bonus: ECB Image Weakness Demo\n";
        std::cout << "   " << RED    << "[4]" << RESET << " Exit\n\n";

        int action = getIntInput("   Select Action: ", 1, 4);

        // Exit condition
        if (action == 4) {
            std::cout << "\n   " << GREEN << "Exiting application. Goodbye!" << RESET << "\n";
            break;
        }

        try {
            // ---------------- ECB IMAGE DEMO (BMP ONLY) ----------------
            if (action == 3) {
                std::cout << "\n   " << YELLOW << "--- ECB WEAKNESS DEMO (BMP ONLY) ---" << RESET << "\n";

                // Ask filenames
                std::string inFile  = getStringInput("   Enter input BMP filename (e.g., tux.bmp): ");
                std::string outFile = getStringInput("   Enter output filename (e.g., tux_ecb.bmp): ");

                showProgressBar("Reading File");
                std::vector<uint8_t> fileData = readFile(inFile);

                // BMP header is typically 54 bytes minimum
                if (fileData.size() < 54) throw std::runtime_error("File too small to be a BMP");

                // Keep header unchanged; encrypt only pixel data
                std::vector<uint8_t> header(fileData.begin(), fileData.begin() + 54);
                std::vector<uint8_t> pixelData(fileData.begin() + 54, fileData.end());

                // Demo key (fixed) for repeatable output
                std::vector<uint8_t> key(16, 0x55);
                AESModes cipher(key.data(), key.size());

                showProgressBar("Encrypting Pixels (ECB)");
                std::vector<uint8_t> encryptedPixels = cipher.encryptECB(pixelData);

                // Rebuild output: header + encrypted pixel bytes
                std::vector<uint8_t> finalOutput = header;
                finalOutput.insert(finalOutput.end(), encryptedPixels.begin(), encryptedPixels.end());

                writeFile(outFile, finalOutput);

                std::cout << "\n   " << GREEN << "[SUCCESS]" << RESET << " Image encrypted with header preserved.\n";
                std::cout << "   Open " << BOLD << outFile << RESET << " to see the ECB patterns!\n";

                waitForKey();
                continue; // go back to main menu
            }

            // ---------------- STANDARD ENCRYPT/DECRYPT ----------------

            // 1) Choose AES key size
            std::cout << "\n   " << BOLD << "--- CONFIGURATION ---" << RESET << "\n";
            std::cout << "   1. AES-128\n   2. AES-192\n   3. AES-256\n";
            int keyChoice = getIntInput("   Select Key Size: ", 1, 3);
            int keySize = (keyChoice == 1) ? 16 : (keyChoice == 2) ? 24 : 32;

            // 2) Choose key source (random or manual hex)
            std::cout << "\n   1. Generate Random Key\n   2. Enter Hex Manually\n";
            int keySource = getIntInput("   Key Source: ", 1, 2);

            std::vector<uint8_t> key(keySize);

            if (keySource == 1) {
                // Generate random key bytes
                CustomRNG rng;
                rng.getBytes(key.data(), keySize);
                std::cout << "   " << GREEN << "Generated Key: " << RESET;
                printHex(key);
            } else {
                // Manual key entry in hex
                std::string hexKey;
                while (true) {
                    hexKey = getStringInput("   Enter Key (Hex): ");
                    if (hexKey.length() == (size_t)keySize * 2) break;
                    std::cout << "   " << RED << "Invalid length! Expected "
                              << keySize * 2 << " hex characters." << RESET << "\n";
                }
                key = hexToBytes(hexKey);
            }

            // Create AESModes object with expanded key
            AESModes cipher(key.data(), key.size());

            // 3) Choose AES mode
            std::cout << "\n   " << BOLD << "--- MODE SELECTION ---" << RESET << "\n";
            std::cout << "   1. ECB (Electronic Codebook)\n";
            std::cout << "   2. CBC (Cipher Block Chaining)\n";
            std::cout << "   3. CTR (Counter Mode)\n";
            std::cout << "   4. GCM (Galois/Counter Mode)\n";
            int modeChoice = getIntInput("   Select Mode: ", 1, 4);

            // 4) Choose input type (console text or file)
            std::cout << "\n   " << BOLD << "--- DATA INPUT ---" << RESET << "\n";
            std::cout << "   1. Text String (Console)\n   2. File (Binary)\n";
            int inputSource = getIntInput("   Input Source: ", 1, 2);

            std::vector<uint8_t> inputData;
            std::string outFilename;

            if (inputSource == 1) {
                // Text mode: encryption -> read plaintext; decryption -> read hex ciphertext
                if (action == 1) {
                    std::string text = getStringInput("   Enter Plaintext: ");
                    inputData.assign(text.begin(), text.end());
                } else {
                    std::string hex = getStringInput("   Enter Ciphertext (Hex): ");
                    inputData = hexToBytes(hex);
                }
            } else {
                // File mode: read bytes from file and ask for output filename
                std::string filename = getStringInput("   Enter Input Filename: ");
                showProgressBar("Reading File");
                inputData = readFile(filename);
                outFilename = getStringInput("   Enter Output Filename: ");
            }

            // 5) Perform operation
            std::vector<uint8_t> outputData;
            std::vector<uint8_t> aad; // Used only for GCM

            // If GCM, ask for AAD (authenticated but not encrypted)
            if (modeChoice == 4) {
                std::string prompt = (action == 1)
                    ? "   Enter AAD (Optional): "
                    : "   Enter AAD used during encryption: ";
                std::string aadStr = getStringInput(prompt);
                aad.assign(aadStr.begin(), aadStr.end());
            }

            std::cout << "\n";
            showProgressBar("Processing Crypto Engine");

            // Measure performance time
            auto start = std::chrono::high_resolution_clock::now();

            if (action == 1) { // ENCRYPT
                if (modeChoice == 4)      outputData = cipher.encryptGCM(inputData, aad);
                else if (modeChoice == 1) outputData = cipher.encryptECB(inputData);
                else if (modeChoice == 2) outputData = cipher.encryptCBC(inputData);
                else if (modeChoice == 3) outputData = cipher.encryptCTR(inputData);
            } else { // DECRYPT
                if (modeChoice == 4)      outputData = cipher.decryptGCM(inputData, aad);
                else if (modeChoice == 1) outputData = cipher.decryptECB(inputData);
                else if (modeChoice == 2) outputData = cipher.decryptCBC(inputData);
                else if (modeChoice == 3) outputData = cipher.decryptCTR(inputData);
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;

            // 6) Print results + performance
            std::cout << "\n   " << GREEN << "--- OPERATION SUCCESSFUL ---" << RESET << "\n";
            std::cout << "   Time elapsed: " << std::fixed << std::setprecision(4)
                      << elapsed.count() << " ms\n";

            // Simple throughput estimate (MB/s)
            std::cout << "   Throughput:   "
                      << (inputData.size() / 1024.0 / 1024.0) / (elapsed.count() / 1000.0)
                      << " MB/s\n";

            // Print or save output
            if (inputSource == 1) {
                if (action == 1) {
                    std::cout << "   Ciphertext (Hex): ";
                    printHex(outputData);
                } else {
                    std::string recovered(outputData.begin(), outputData.end());
                    std::cout << "   Recovered Plaintext: " << recovered << "\n";
                }
            } else {
                writeFile(outFilename, outputData);
                std::cout << "   Output saved to " << BOLD << outFilename << RESET << "\n";
            }

        } catch (const std::exception& e) {
            // Catch any errors (bad padding, invalid tag, wrong input size, file errors, etc.)
            std::cout << "\n   " << RED << "[ERROR]: " << e.what() << RESET << "\n";
        }

        waitForKey();
    }

    return 0;
}
