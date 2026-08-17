// auth.hpp
// --------
// Small, self-contained helpers for:
//   1. Hashing passwords (so we never store them as plain text)
//   2. Generating random tokens for login sessions
//
// This includes a from-scratch implementation of the SHA-256 algorithm
// (a well-known, public hashing standard - not a copy of any specific
// library's code) so the project has no extra dependency just for this.
//
// NOTE FOR BEGINNERS: a real production app should use a purpose-built
// password-hashing algorithm like bcrypt or Argon2 (they're deliberately
// slow, which protects against attackers guessing passwords quickly).
// Plain SHA-256 + a random salt, as used here, is fine for learning and
// demo purposes, but keep that limitation in mind.

#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace sha256_impl {

inline uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

// Computes the SHA-256 hash of `input` and returns it as a lowercase hex
// string. This follows the standard SHA-256 algorithm (FIPS 180-4).
inline std::string sha256_hex(const std::string& input) {
    // The 64 round constants defined by the SHA-256 standard.
    static const uint32_t k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    // Initial hash values (first 32 bits of the fractional parts of the
    // square roots of the first 8 prime numbers).
    uint32_t h[8] = {
        0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
        0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
    };

    // --- Step 1: pad the message ---
    std::vector<uint8_t> data(input.begin(), input.end());
    uint64_t bit_length = static_cast<uint64_t>(data.size()) * 8;

    data.push_back(0x80); // append a single '1' bit (as 0x80 byte)
    while (data.size() % 64 != 56) {
        data.push_back(0x00);
    }
    for (int i = 7; i >= 0; --i) {
        data.push_back(static_cast<uint8_t>((bit_length >> (i * 8)) & 0xff));
    }

    // --- Step 2: process the message in 64-byte chunks ---
    for (size_t chunk = 0; chunk < data.size(); chunk += 64) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(data[chunk + i * 4]) << 24) |
                   (static_cast<uint32_t>(data[chunk + i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(data[chunk + i * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(data[chunk + i * 4 + 3]));
        }
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = hh + S1 + ch + k[i] + w[i];
            uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;

            hh = g; g = f; f = e; e = d + temp1;
            d = c; c = b; b = a; a = temp1 + temp2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    // --- Step 3: produce the final hex string ---
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (int i = 0; i < 8; ++i) {
        out << std::setw(8) << h[i];
    }
    return out.str();
}

} // namespace sha256_impl

// Generates a random hex string of the given length (used for both salts
// and session tokens). Uses std::random_device, a source of randomness
// suitable for this kind of use.
inline std::string generate_random_hex(size_t length) {
    static const char hex_chars[] = "0123456789abcdef";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, 15);

    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += hex_chars[distribution(generator)];
    }
    return result;
}

// Hashes a plain-text password together with a freshly generated random
// salt. Returns a single string in the form "salt$hash" so it can be
// stored as one column in the database.
inline std::string hash_password(const std::string& plain_password) {
    std::string salt = generate_random_hex(16);
    std::string hash = sha256_impl::sha256_hex(salt + plain_password);
    return salt + "$" + hash;
}

// Checks a plain-text password against a stored "salt$hash" string.
inline bool verify_password(const std::string& plain_password, const std::string& stored_hash) {
    size_t separator = stored_hash.find('$');
    if (separator == std::string::npos) {
        return false;
    }
    std::string salt = stored_hash.substr(0, separator);
    std::string expected_hash = stored_hash.substr(separator + 1);
    std::string actual_hash = sha256_impl::sha256_hex(salt + plain_password);
    return actual_hash == expected_hash;
}

// Generates a new random session token (used as the value of the
// "session_token" cookie once a user logs in).
inline std::string generate_session_token() {
    return generate_random_hex(32);
}
