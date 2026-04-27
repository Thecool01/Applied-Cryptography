#pragma once
#include <vector>
#include <cstdint>
#include <stdexcept>
#include "sha256.hpp"
#include "padding.hpp"

// Forward declaration — the RSA core is provided by Developer 1.
// This header expects the following free functions to exist (from rsa_core.hpp):
//
//   BigInt rsaRawPrivate(const BigInt& m, const RSAPrivateKey& priv);
//   BigInt rsaRawPublic (const BigInt& m, const RSAPublicKey&  pub);
//   size_t  rsaModulusBytes(const RSAPublicKey& pub);
//   BigInt  bytesToBigInt(const std::vector<uint8_t>&);
//   std::vector<uint8_t> bigIntToBytes(const BigInt&, size_t len);
//
// Include rsa_core.hpp (Developer 1's header) before this file.

/**
 * RSA Digital Signature — Developer 2 module
 *
 * Supports two schemes:
 *   1. PKCS#1 v1.5 signatures (classic, FIPS 186-4 compatible)
 *   2. PSS signatures         (BONUS — probabilistic, stronger)
 */
template<typename BigInt, typename RSAPrivKey, typename RSAPubKey,
         typename RawPrivFn, typename RawPubFn,
         typename B2BIFn, typename BI2BFn>
class RSASigner {
public:
    RSASigner(RawPrivFn rawPriv, RawPubFn rawPub, B2BIFn b2bi, BI2BFn bi2b)
        : rawPriv_(rawPriv), rawPub_(rawPub), b2bi_(b2bi), bi2b_(bi2b) {}

    // ── PKCS#1 v1.5 Sign ─────────────────────────────────────────
    std::vector<uint8_t> signPKCS1(const std::vector<uint8_t>& message,
                                   const RSAPrivKey& priv, size_t k) {
        auto digest = SHA256::hash(message);
        auto EM     = PKCS1v15::sigPad(digest, k);
        auto mInt   = b2bi_(EM);
        auto sInt   = rawPriv_(mInt, priv);
        return bi2b_(sInt, k);
    }

    // ── PKCS#1 v1.5 Verify ───────────────────────────────────────
    bool verifyPKCS1(const std::vector<uint8_t>& message,
                     const std::vector<uint8_t>& signature,
                     const RSAPubKey& pub, size_t k) {
        try {
            auto sInt    = b2bi_(signature);
            auto emInt   = rawPub_(sInt, pub);
            auto EM      = bi2b_(emInt, k);
            auto T       = PKCS1v15::sigUnpad(EM);
            auto recovered = PKCS1v15::extractDigest(T);
            auto computed  = SHA256::hash(message);
            return recovered == computed;
        } catch (...) {
            return false;
        }
    }

    // ── PSS Sign (BONUS) ─────────────────────────────────────────
    std::vector<uint8_t> signPSS(const std::vector<uint8_t>& message,
                                 const RSAPrivKey& priv, size_t k) {
        auto digest = SHA256::hash(message);
        size_t emBits = k * 8 - 1;
        auto EM   = PSS::encode(digest, emBits);
        auto mInt = b2bi_(EM);
        auto sInt = rawPriv_(mInt, priv);
        return bi2b_(sInt, k);
    }

    // ── PSS Verify (BONUS) ───────────────────────────────────────
    bool verifyPSS(const std::vector<uint8_t>& message,
                   const std::vector<uint8_t>& signature,
                   const RSAPubKey& pub, size_t k) {
        try {
            auto sInt  = b2bi_(signature);
            auto emInt = rawPub_(sInt, pub);
            auto EM    = bi2b_(emInt, k);
            auto digest = SHA256::hash(message);
            size_t emBits = k * 8 - 1;
            return PSS::verify(digest, EM, emBits);
        } catch (...) {
            return false;
        }
    }

private:
    RawPrivFn rawPriv_;
    RawPubFn  rawPub_;
    B2BIFn    b2bi_;
    BI2BFn    bi2b_;
};
