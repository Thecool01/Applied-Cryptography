# RSA Cryptosystem Implementation (C++)

## Students:
> Urazakov Omar 24B032086 
> 
> Ishutin Nikolay 24B031822 

This project is a complete implementation of the RSA public-key cryptosystem from scratch, developed as part of the **Applied Cryptography (SIS3)** course[cite: 1, 2, 6]. [cite_start]It includes prime number generation, key pair generation, encryption/decryption, and digital signatures[cite: 6].

## 🚀 Key Features

* **Custom Math Engine**: Implementation of Modular Exponentiation (Square-and-Multiply), GCD, and Extended Euclidean Algorithm[cite: 63, 74, 80].
* **Secure Prime Generation**: Generates 512, 1024, and 2048-bit primes using a **Custom RNG** and the Miller-Rabin primality test (min. 40-64 iterations)[cite: 34, 37, 43].
* **Standard Key Sizes**: Supports 1024, 2048, and 4096-bit RSA keys[cite: 108, 111].
* **Advanced Encryption**: Supports PKCS#1 v1.5 and OAEP padding schemes[cite: 120, 128].
* **Digital Signatures**: Custom SHA-256 implementation with PKCS#1 v1.5 and **PSS (Bonus)** padding[cite: 147, 154, 279].
* **Optimized Performance**: Features **CRT (Chinese Remainder Theorem)** for faster decryption[cite: 278].

## 🛠 Project Structure

* `BigIntMath.h/.cpp` — Core modular arithmetic functions.
* `CustomRNG.h` — Cryptographically strong random number generator.
* `PrimeGenerator.h/.cpp` — Prime search and Miller-Rabin test logic.
* `RSACore.h/.cpp` — Key generation and raw RSA operations.
* `sha256.hpp` — SHA-256 hash function implementation from scratch.
* `padding.hpp` — PKCS#1 v1.5, OAEP, and PSS padding schemes.
* `rsa_signatures.hpp` — High-level digital signature management.
* `main.cpp` — Interactive CLI with defense-mode (default values).

## 📦 Prerequisites

[cite_start]The project requires the **Boost.Multiprecision** library for handling large integers (BigInt).
* Ensure Boost is installed or available in your include path (e.g., `C:\local\boost_1_90_0`).

## 🔨 Build Instructions (VS Code)

1.  Open the project folder in **VS Code**.
2.  Ensure your `.vscode/tasks.json` is configured to include the Boost path and compile all `.cpp` files:
    ```json
    "args": ["-g", "-I", "C:\\local\\boost_1_90_0", "*.cpp", "-o", "main.exe"]
    ```
3.  Press `Ctrl + Shift + B` to build the project.
4.  Run the application using:
    ```bash
    ./main.exe
    ```

## 🖥 Usage (Defense Mode)

The application features an interactive menu. For a quick demonstration during defense:
1.  **Generate Keys**: Press `1` then `Enter`. (Default size 1024-bit).
2.  **Encrypt**: Press `2` then `Enter`. (Uses default test message).
3.  **Decrypt**: Press `3` then `Enter`. (Auto-detects the last ciphertext).
4.  **Sign**: Press `4` then `Enter`.
5.  **Verify**: Press `5` then `Enter`.
