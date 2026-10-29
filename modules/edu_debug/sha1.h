/******************************************************************************
 * Self-contained SHA-1 used to fingerprint configuration files so that the
 * value printed at module start can be compared with `sha1sum` on the host.
 *****************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

namespace apollo {
namespace edu_debug {

class Sha1 {
 public:
  Sha1() { Reset(); }

  void Reset() {
    h_[0] = 0x67452301u;
    h_[1] = 0xEFCDAB89u;
    h_[2] = 0x98BADCFEu;
    h_[3] = 0x10325476u;
    h_[4] = 0xC3D2E1F0u;
    bit_length_ = 0;
    buffer_len_ = 0;
  }

  void Update(const void* data, std::size_t len) {
    const std::uint8_t* input = static_cast<const std::uint8_t*>(data);
    bit_length_ += static_cast<std::uint64_t>(len) * 8u;
    while (len > 0) {
      const std::size_t take = 64 - buffer_len_;
      const std::size_t n = len < take ? len : take;
      std::memcpy(buffer_ + buffer_len_, input, n);
      buffer_len_ += n;
      input += n;
      len -= n;
      if (buffer_len_ == 64) {
        ProcessBlock(buffer_);
        buffer_len_ = 0;
      }
    }
  }

  std::string HexDigest() {
    const std::uint64_t saved_length = bit_length_;
    const std::uint8_t one = 0x80;
    Update(&one, 1);
    const std::uint8_t zero = 0x00;
    while (buffer_len_ != 56) {
      Update(&zero, 1);
    }
    std::uint8_t length_bytes[8];
    for (int i = 0; i < 8; ++i) {
      length_bytes[7 - i] =
          static_cast<std::uint8_t>((saved_length >> (i * 8)) & 0xFFu);
    }
    Update(length_bytes, 8);
    char out[41];
    for (int i = 0; i < 5; ++i) {
      std::snprintf(out + i * 8, 9, "%08x", h_[i]);
    }
    Reset();
    return std::string(out, 40);
  }

 private:
  static std::uint32_t Rotl(std::uint32_t value, int bits) {
    return (value << bits) | (value >> (32 - bits));
  }

  void ProcessBlock(const std::uint8_t* block) {
    std::uint32_t w[80];
    for (int i = 0; i < 16; ++i) {
      w[i] = (static_cast<std::uint32_t>(block[i * 4]) << 24) |
             (static_cast<std::uint32_t>(block[i * 4 + 1]) << 16) |
             (static_cast<std::uint32_t>(block[i * 4 + 2]) << 8) |
             (static_cast<std::uint32_t>(block[i * 4 + 3]));
    }
    for (int i = 16; i < 80; ++i) {
      w[i] = Rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }
    std::uint32_t a = h_[0];
    std::uint32_t b = h_[1];
    std::uint32_t c = h_[2];
    std::uint32_t d = h_[3];
    std::uint32_t e = h_[4];
    for (int i = 0; i < 80; ++i) {
      std::uint32_t f = 0;
      std::uint32_t k = 0;
      if (i < 20) {
        f = (b & c) | ((~b) & d);
        k = 0x5A827999u;
      } else if (i < 40) {
        f = b ^ c ^ d;
        k = 0x6ED9EBA1u;
      } else if (i < 60) {
        f = (b & c) | (b & d) | (c & d);
        k = 0x8F1BBCDCu;
      } else {
        f = b ^ c ^ d;
        k = 0xCA62C1D6u;
      }
      const std::uint32_t temp = Rotl(a, 5) + f + e + k + w[i];
      e = d;
      d = c;
      c = Rotl(b, 30);
      b = a;
      a = temp;
    }
    h_[0] += a;
    h_[1] += b;
    h_[2] += c;
    h_[3] += d;
    h_[4] += e;
  }

  std::uint32_t h_[5];
  std::uint64_t bit_length_;
  std::uint8_t buffer_[64];
  std::size_t buffer_len_;
};

}  // namespace edu_debug
}  // namespace apollo
