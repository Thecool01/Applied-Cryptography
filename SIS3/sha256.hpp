#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>

/**
 * SHA-256 from scratch — FIPS 180-4
 */
class SHA256 {
public:
    static constexpr size_t DIGEST_SIZE = 32;

    static std::vector<uint8_t> hash(const std::vector<uint8_t>& data) {
        SHA256 ctx; ctx.update(data.data(), data.size()); return ctx.finalize();
    }
    static std::vector<uint8_t> hash(const std::string& s) {
        return hash(std::vector<uint8_t>(s.begin(), s.end()));
    }
    static std::string toHex(const std::vector<uint8_t>& d) {
        std::ostringstream o;
        for (auto b : d) o << std::hex << std::setw(2) << std::setfill('0') << (int)b;
        return o.str();
    }

    SHA256() { reset(); }

    void reset() {
        H[0]=0x6a09e667; H[1]=0xbb67ae85; H[2]=0x3c6ef372; H[3]=0xa54ff53a;
        H[4]=0x510e527f; H[5]=0x9b05688c; H[6]=0x1f83d9ab; H[7]=0x5be0cd19;
        total_bits=0; buf_len=0;
    }

    void update(const uint8_t* data, size_t len) {
        total_bits += (uint64_t)len * 8;
        size_t off = 0;
        if (buf_len > 0) {
            size_t take = 64 - buf_len; if (take > len) take = len;
            memcpy(buf + buf_len, data, take);
            buf_len += take; off += take;
            if (buf_len == 64) { compress(buf); buf_len = 0; }
        }
        while (off + 64 <= len) { compress(data + off); off += 64; }
        if (off < len) { memcpy(buf, data + off, len - off); buf_len = len - off; }
    }

    std::vector<uint8_t> finalize() {
        uint64_t bits = total_bits;       // save before padding alters total_bits
        // Padding: append 0x80, zeros, then 64-bit big-endian length
        uint8_t pad[64] = {};
        size_t padStart = buf_len;
        // We need to manually pad without using update() to avoid corrupting total_bits
        pad[0] = 0x80;
        if (padStart < 56) {
            // Fits in one block
            memcpy(buf + padStart, pad, 56 - padStart + 1);
            buf_len = 56;
        } else {
            // Need two blocks
            memcpy(buf + buf_len, pad, 64 - buf_len);
            compress(buf);
            memset(buf, 0, 56);
            buf_len = 56;
        }
        // Append 64-bit big-endian bit count
        buf[56] = (bits >> 56) & 0xff; buf[57] = (bits >> 48) & 0xff;
        buf[58] = (bits >> 40) & 0xff; buf[59] = (bits >> 32) & 0xff;
        buf[60] = (bits >> 24) & 0xff; buf[61] = (bits >> 16) & 0xff;
        buf[62] = (bits >>  8) & 0xff; buf[63] =  bits        & 0xff;
        compress(buf);

        std::vector<uint8_t> out(32);
        for (int i = 0; i < 8; ++i) {
            out[i*4+0]=(H[i]>>24)&0xff; out[i*4+1]=(H[i]>>16)&0xff;
            out[i*4+2]=(H[i]>> 8)&0xff; out[i*4+3]= H[i]     &0xff;
        }
        reset();
        return out;
    }

private:
    uint32_t H[8];
    uint64_t total_bits;
    uint8_t  buf[64];
    size_t   buf_len;

    static constexpr uint32_t K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    static uint32_t rotr(uint32_t x,int n){return(x>>n)|(x<<(32-n));}
    static uint32_t Ch (uint32_t x,uint32_t y,uint32_t z){return(x&y)^(~x&z);}
    static uint32_t Maj(uint32_t x,uint32_t y,uint32_t z){return(x&y)^(x&z)^(y&z);}
    static uint32_t S0(uint32_t x){return rotr(x,2)^rotr(x,13)^rotr(x,22);}
    static uint32_t S1(uint32_t x){return rotr(x,6)^rotr(x,11)^rotr(x,25);}
    static uint32_t s0(uint32_t x){return rotr(x,7)^rotr(x,18)^(x>>3);}
    static uint32_t s1(uint32_t x){return rotr(x,17)^rotr(x,19)^(x>>10);}

    void compress(const uint8_t* blk) {
        uint32_t W[64];
        for(int i=0;i<16;++i)
            W[i]=(uint32_t(blk[i*4])<<24)|(uint32_t(blk[i*4+1])<<16)
                |(uint32_t(blk[i*4+2])<<8)|blk[i*4+3];
        for(int i=16;i<64;++i) W[i]=s1(W[i-2])+W[i-7]+s0(W[i-15])+W[i-16];
        uint32_t a=H[0],b=H[1],c=H[2],d=H[3],e=H[4],f=H[5],g=H[6],h=H[7];
        for(int i=0;i<64;++i){
            uint32_t T1=h+S1(e)+Ch(e,f,g)+K[i]+W[i];
            uint32_t T2=S0(a)+Maj(a,b,c);
            h=g;g=f;f=e;e=d+T1;d=c;c=b;b=a;a=T1+T2;
        }
        H[0]+=a;H[1]+=b;H[2]+=c;H[3]+=d;H[4]+=e;H[5]+=f;H[6]+=g;H[7]+=h;
    }
};
constexpr uint32_t SHA256::K[64];
