#pragma once

#include <algorithm>

struct MemoryChunk {
  size_t size = 0;
  size_t capacity = 0;
  char *data = nullptr;

  // Methods
  void Clear() {
    size = capacity = 0;
    if (data) {
      delete[] data;
    }
  }

  // Ctrs
  explicit MemoryChunk(size_t n = 0)
      : size(0), capacity(n), data(capacity ? new char[capacity] : nullptr) {
    if (data) {
      std::fill(data, data + capacity, 0);
    }
  }

  MemoryChunk(const MemoryChunk &other)
      : size(other.size),
        capacity(other.capacity),
        data(capacity ? new char[capacity] : nullptr) {
    std::copy(other.data, other.data + other.capacity, data);
  }

  MemoryChunk &operator=(const MemoryChunk &other) {
    if (this != &other) {
      Clear();
      size = other.size;
      capacity = other.capacity;
      data = capacity ? new char[capacity] : nullptr;
      std::copy(other.data, other.data + other.capacity, data);
    }
    return *this;
  }

  MemoryChunk(MemoryChunk &&other) noexcept
      : size(other.size),
        capacity(other.capacity),
        data(capacity ? other.data : nullptr) {
    other.size = 0;
    other.capacity = 0;
    other.data = nullptr;
  }

  MemoryChunk &operator=(MemoryChunk &&other) noexcept {
    if (this != &other) {
      Clear();
      size = other.size;
      capacity = other.capacity;
      data = capacity ? other.data : nullptr;
      other.size = 0;
      other.capacity = 0;
      other.data = nullptr;
    }
    return *this;
  }

  ~MemoryChunk() { Clear(); }
};