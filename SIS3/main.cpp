#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <sstream>

// Включаем ваши модули
#include "BigIntMath.h"
#include "PrimeGenerator.h"
#include "RSACore.h"
#include "sha256.hpp"
#include "padding.hpp"
#include "rsa_signatures.hpp"

using namespace std;
using namespace boost::multiprecision;

// --- Вспомогательные функции ---

std::vector<uint8_t> biToBytes(const cpp_int& bi, size_t len) {
    std::vector<uint8_t> v;
    export_bits(bi, std::back_inserter(v), 8);
    if (v.size() < len) {
        v.insert(v.begin(), len - v.size(), 0);
    }
    return v;
}

cpp_int bytesToBi(const std::vector<uint8_t>& v) {
    cpp_int bi;
    import_bits(bi, v.begin(), v.end());
    return bi;
}

cpp_int rawPub(const cpp_int& m, const PublicKey& pub) { return RSACore::encryptTextbook(m, pub); }
cpp_int rawPriv(const cpp_int& c, const PrivateKey& priv) { return RSACore::decryptCRT(c, priv); }

// Вспомогательная функция для перевода cpp_int в hex строку
string toHexStr(const cpp_int& val) {
    stringstream ss;
    ss << hex << val;
    return ss.str();
}

void printMenu() {
    cout << "\n========== RSA CRYPTOSYSTEM MENU ==========" << endl;
    cout << "1. Generate New Key Pair" << endl;
    cout << "2. Encrypt Message (PKCS#1 v1.5)" << endl;
    cout << "3. Decrypt Message (CRT + Padding Check)" << endl;
    cout << "4. Sign Message (PSS Bonus)" << endl;
    cout << "5. Verify Signature" << endl;
    cout << "6. Exit" << endl;
    cout << "Selection: ";
}

int main() {
    PublicKey pubKey;
    PrivateKey privKey;
    bool keysGenerated = false;

    // Глобальные переменные для авто-заполнения на защите
    string defaultMessage = "Hello, Professor! This is our RSA defense.";
    string lastCiphertextHex = "";
    string lastSignatureHex = "";

    RSASigner<cpp_int, PrivateKey, PublicKey, 
              decltype(&rawPriv), decltype(&rawPub), 
              decltype(&bytesToBi), decltype(&biToBytes)> 
    signer(rawPriv, rawPub, bytesToBi, biToBytes);

    string input;
    int choice;

    while (true) {
        printMenu();
        getline(cin, input);
        if (input.empty()) continue;
        
        try { choice = stoi(input); } 
        catch (...) { cout << "Invalid input." << endl; continue; }

        if (choice == 6) break;

        switch (choice) {
            case 1: {
                cout << "Choose key size (1024, 2048, 4096) [Press Enter for 1024]: ";
                getline(cin, input);
                int size = input.empty() ? 1024 : stoi(input);
                
                auto start = chrono::high_resolution_clock::now();
                RSACore::generateKeys(size, pubKey, privKey);
                auto end = chrono::high_resolution_clock::now();
                auto diff = chrono::duration_cast<chrono::seconds>(end - start).count();
                
                cout << "\n[+] Keys generated in " << diff << " seconds." << endl;
                cout << "[+] Modulus (hex): " << hex << pubKey.n << dec << "\n" << endl;
                keysGenerated = true;
                break;
            }
            case 2: {
                if (!keysGenerated) { cout << "[-] Generate keys first!" << endl; break; }
                
                cout << "Enter message to encrypt [Press Enter for default]: ";
                string msg;
                getline(cin, msg);
                if (msg.empty()) msg = defaultMessage;

                vector<uint8_t> data(msg.begin(), msg.end());
                size_t k = (msb(pubKey.n) + 1 + 7) / 8;
                
                auto em = PKCS1v15::encPad(data, k);
                cpp_int mInt = bytesToBi(em);
                cpp_int cInt = RSACore::encryptTextbook(mInt, pubKey);
                
                lastCiphertextHex = toHexStr(cInt); // Сохраняем для автозаполнения
                
                cout << "\n[+] Ciphertext (hex): " << lastCiphertextHex << "\n" << endl;
                break;
            }
            case 3: {
                if (!keysGenerated) { cout << "[-] Generate keys first!" << endl; break; }
                
                cout << "Enter ciphertext (hex) [Press Enter to use last generated]: ";
                string hexC;
                getline(cin, hexC);
                if (hexC.empty()) hexC = lastCiphertextHex;
                if (hexC.empty()) { cout << "[-] No ciphertext available!" << endl; break; }

                cpp_int cInt("0x" + hexC);
                cpp_int mInt = RSACore::decryptCRT(cInt, privKey);
                size_t k = (msb(pubKey.n) + 1 + 7) / 8;
                vector<uint8_t> em = biToBytes(mInt, k);
                
                try {
                    auto decData = PKCS1v15::encUnpad(em);
                    string decrypted(decData.begin(), decData.end());
                    cout << "\n[+] Decrypted message: " << decrypted << "\n" << endl;
                } catch (exception& e) {
                    cout << "\n[-] Padding error or wrong key!" << "\n" << endl;
                }
                break;
            }
            case 4: {
                if (!keysGenerated) { cout << "[-] Generate keys first!" << endl; break; }
                
                cout << "Enter message to sign [Press Enter for default]: ";
                string msg;
                getline(cin, msg);
                if (msg.empty()) msg = defaultMessage;

                vector<uint8_t> data(msg.begin(), msg.end());
                size_t k = (msb(pubKey.n) + 1 + 7) / 8;
                
                auto sig = signer.signPSS(data, privKey, k);
                lastSignatureHex = SHA256::toHex(sig); // Сохраняем для автозаполнения
                
                cout << "\n[+] Signature (hex): " << lastSignatureHex << "\n" << endl;
                break;
            }
            case 5: {
                if (!keysGenerated) { cout << "[-] Generate keys first!" << endl; break; }
                
                cout << "Enter original message [Press Enter for default]: ";
                string msg;
                getline(cin, msg);
                if (msg.empty()) msg = defaultMessage;

                cout << "Enter signature (hex) [Press Enter to use last generated]: ";
                string sigHex;
                getline(cin, sigHex);
                if (sigHex.empty()) sigHex = lastSignatureHex;
                if (sigHex.empty()) { cout << "[-] No signature available!" << endl; break; }

                vector<uint8_t> sig;
                for (size_t i = 0; i < sigHex.length(); i += 2) {
                    sig.push_back((uint8_t)stoul(sigHex.substr(i, 2), nullptr, 16));
                }

                vector<uint8_t> data(msg.begin(), msg.end());
                size_t k = (msb(pubKey.n) + 1 + 7) / 8;
                
                if (signer.verifyPSS(data, sig, pubKey, k)) {
                    cout << "\n[+] VERIFICATION SUCCESSFUL: Signature is valid.\n" << endl;
                } else {
                    cout << "\n[-] VERIFICATION FAILED: Invalid signature.\n" << endl;
                }
                break;
            }
            default: cout << "[-] Invalid choice." << endl;
        }
    }
    return 0;
}