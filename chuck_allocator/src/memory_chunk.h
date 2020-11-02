#pragma once

#include <algorithm>

struct MemoryChunk {
  size_t size = 0;
  size_t capacity = 0;
  uint8_t *data = nullptr;

  // Methods
  void Clear() {
    size = capacity = 0;
    if (data) {
      delete[] data;
    }
  }

  uint8_t *Allocate(uint8_t *other = nullptr) {
    return capacity > size_t(0) ? (other ? other : new uint8_t[capacity])
                                : static_cast<uint8_t *>(nullptr);
  }

  // Ctrs
  explicit MemoryChunk(size_t n = 0)
      : size(0),
        capacity(n),
        data(Allocate()) {
    if (data) {
      std::fill(data, data + capacity, uint8_t(0));
    }
  }

  MemoryChunk(const MemoryChunk &other)
      : size(other.size),
        capacity(other.capacity),
        data(Allocate()) {
    std::copy(other.data, other.data + other.capacity, data);
  }

  MemoryChunk &operator=(const MemoryChunk &other) {
    if (this != &other) {
      Clear();
      size = other.size;
      capacity = other.capacity;
      data = Allocate();
      std::copy(other.data, other.data + other.capacity, data);
    }
    return *this;
  }

  MemoryChunk(MemoryChunk &&other) noexcept
      : size(other.size),
        capacity(other.capacity),
        data(Allocate(other.data)) {
    other.size = 0;
    other.capacity = 0;
    other.data = static_cast<uint8_t *>(nullptr);
  }

  MemoryChunk &operator=(MemoryChunk &&other) noexcept {
    if (this != &other) {
      Clear();
      size = other.size;
      capacity = other.capacity;
      data = Allocate(other.data);
      other.size = 0;
      other.capacity = 0;
      other.data = static_cast<uint8_t *>(nullptr);
    }
    return *this;
  }

  ~MemoryChunk() { Clear(); }
};