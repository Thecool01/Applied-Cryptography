# IS1 – AES Implementation (C++)

This project is a **student implementation of the AES block cipher** with multiple modes of operation.

It was developed as part of **Student Independent Study 1 (SIS1)** to understand how AES works internally and how different encryption modes behave in practice.

The project includes:

- AES core implementation (AES-128 / AES-192 / AES-256)
- Modes of operation: **ECB, CBC, CTR, GCM**
- PKCS#7 padding
- Custom pseudo-random generator (for IV / nonce)
- CLI application for encryption and decryption
- Automated test suite with **NIST test vectors**

---

## Project Structure

```
.
├── aes.h / aes.cpp# AES core (key expansion, encrypt/decrypt block)
├── AESModes.h# AES modes: ECB, CBC, CTR, GCM
├── Padding.h# PKCS#7 padding
├── CustomRNG.h# Simple PRNG for IV/nonce/key generation
├── GCM_Math.h# GF(2^128) math for GCM (GHASH)
├── main.cpp# CLI application
├── test.cpp# Automated test suite (NIST + edge cases)
└── README.md
```

---

## Requirements

- C++ compiler with **C++17 support**
- Tested with:
    - `g++` (Linux / MinGW / MSYS2)
- Operating systems:
    - Windows
    - Linux
    - macOS (should work)

---

## Build Instructions

### 1. Compile the main CLI application

This builds the interactive encryption/decryption program.

```bash
g++ -std=c++17 -O2 -Wall -Wextra aes.cpp main.cpp -o aes_app
```

On Windows (MinGW):

```bash
g++ -std=c++17 -O2 -Wall -Wextra aes.cpp main.cpp -o aes_app.exe
```

---

### 2. Compile the test suite (recommended)

This builds a **separate executable** for testing the correctness of the implementation.

```bash
g++ -std=c++17 -O2 -Wall -Wextra aes.cpp test.cpp -otest
```

On Windows:

```bash
g++ -std=c++17 -O2 -Wall -Wextra aes.cpp test.cpp -o test.exe
```

---

## Running the Program

### Run the main application

```bash
./aes_app
```

or on Windows:

```bash
aes_app.exe
```

You will see a **CLI menu** where you can:

1. Encrypt data
2. Decrypt data
3. Run ECB image weakness demo (BMP files)
4. Exit

---

## How to Use (Step by Step)

### Encryption

1. Select **Encrypt**
2. Choose AES key size:
    - AES-128
    - AES-192
    - AES-256
3. Choose key source:
    - Generate random key
    - Enter key manually (hex)
4. Select encryption mode:
    - ECB
    - CBC
    - CTR
    - GCM
5. Choose input type:
    - Text (console)
    - File (binary)
6. (For GCM) Enter AAD if needed
7. Program outputs:
    - Ciphertext (hex) for text
    - Encrypted file for file input
    - Execution time and throughput

---

### Decryption

1. Select **Decrypt**
2. Use the same key and mode as encryption
3. For GCM, enter the **same AAD**
4. Program outputs:
    - Recovered plaintext
    - Or decrypted file

---

## ECB Image Weakness Demo

The project includes a **visual demonstration of why ECB is insecure**.

Steps:

1. Select **Bonus: ECB Image Weakness Demo**
2. Provide a **BMP image file**
3. The program encrypts pixel data using ECB
4. The image structure remains visible, showing ECB patterns

This demonstrates why ECB should not be used in real systems.

---

## Running Tests

After compiling `test.cpp`, run:

```bash
./test
```

or on Windows:

```bash
test.exe
```

The test suite performs:

- PKCS#7 padding tests
- Custom RNG sanity tests
- **Official NIST AES test vectors**
- Round-trip tests for all modes
- Edge case tests (empty input, exact block size)
- GCM tamper detection (ciphertext, AAD, tag)
- Large data tests (2 MB)

If everything is correct, you will see:

```
ALL TESTS PASSED SUCCESSFULLY
```

---