# Cryptography Project – SIS2

## Overview

This project implements several modern cryptographic primitives in C++.

The goal of the project is to demonstrate how secure hashing, authentication, and key derivation algorithms work in practice.

The implementation includes:

- SHA-256
- SHA-512
- HMAC-SHA256
- PBKDF2
- HKDF
- File integrity verification
- Password hashing system

These components together demonstrate how cryptographic techniques can be used to protect data integrity, authentication, and password security.

---

# Implemented Algorithms

## SHA-256

SHA-256 is a cryptographic hash function from the SHA-2 family that produces a 256-bit hash value.

Main characteristics:

- fixed output size (256 bits)
- strong collision resistance
- avalanche effect
- widely used in modern security systems

In this project SHA-256 is used for:

- hashing messages
- file integrity verification
- building HMAC

---

## SHA-512

SHA-512 is another member of the SHA-2 family that produces a 512-bit hash.

It is similar to SHA-256 but:

- uses **64-bit operations**
- processes **1024-bit blocks**
- runs **80 compression rounds**

The implementation demonstrates how larger word sizes increase cryptographic strength.

---

## HMAC-SHA256

HMAC (Hash-based Message Authentication Code) is used to verify both message integrity and authenticity.

The algorithm works as follows:

1. Normalize the key to the hash block size.
2. Create inner and outer padded keys.
3. Compute the inner hash.
4. Compute the outer hash.

The result is a secure authentication code that depends on both the message and the secret key.

---

## PBKDF2

PBKDF2 (Password-Based Key Derivation Function 2) derives cryptographic keys from passwords.

Features:

- uses HMAC-SHA256 internally
- applies many iterations
- increases resistance to brute-force attacks

The algorithm repeatedly hashes the password and salt to produce a strong derived key.

---

## HKDF

HKDF (HMAC-based Key Derivation Function) derives secure keys from input key material.

The process consists of two stages:

### Extract

Creates a pseudorandom key using HMAC.

### Expand

Generates the final key material using iterative HMAC operations.

HKDF is widely used in protocols such as TLS.

---

# Additional Components

## File Integrity Verification

The project includes a module for verifying file integrity.

The system:

1. Computes the SHA-256 hash of a file.
2. Stores the hash.
3. Recomputes the hash later to detect modifications.

If the hashes match, the file has not been changed.

---

## Password Manager

The project also demonstrates secure password storage.

Instead of storing raw passwords, the system:

1. Generates a random salt.
2. Uses HKDF to derive a password hash.
3. Stores the salt and derived key.

During login, the same process is applied to verify the password.

This approach protects against:

- rainbow table attacks
- password database leaks

---

# Project Structure

Example project structure:

```
SIS2/src
│
├── sha-256.cpp
├── sha-256.h
├── sha-512.cpp
├── sha-512.h
│
├── hmac.cpp
├── hmac.h
│
├── pbkdf2.cpp
├── pbkdf2.h
│
├── hkdf.cpp
├── hkdf.h
│
├── file_integrity.cpp
├── password_manager.cpp
│
└── main.cpp
```

Each module implements a specific cryptographic functionality.

---

# Build the Project

Compile all source files using the following command:

```
g++-std=c++17-O2-Wall-Wextra \
sha-256.cpp sha-512.cpp hmac.cpp pbkdf2.cpp hkdf.cpp \
file_integrity.cpp password_manager.cpp main.cpp \
-o crypto_project
```


---
# Security Considerations

The project demonstrates several important security principles:

- secure cryptographic hash functions
- message authentication using HMAC
- password hashing with salt
- key derivation functions
- file integrity verification

These techniques are commonly used in real-world systems such as TLS, password managers, and secure communication protocols.

---

# Conclusion

This project demonstrates how fundamental cryptographic algorithms can be implemented and used together to build secure systems.

It shows practical applications of:

- hashing
- authentication
- key derivation
- password protection
- data integrity verification

The implementation provides a clear educational example of how modern cryptography is used in real-world applications.