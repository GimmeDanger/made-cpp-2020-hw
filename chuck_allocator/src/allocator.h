#pragma once

#include <cstddef>

#include "memory_chunk.h"
#include "singly_linked_list.h"

template <class T>
class Allocator {
  // Member types
 public:
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  template <class U>
  struct rebind {
    using other = Allocator<U>;
  };

  // Member data
 private:
  // enough for ~125'000 doubles or ~250'000 integers
  static constexpr size_t MAX_CHUNK_SIZE_DEFAULT = 1'000'000;

  size_t max_chunk_size = MAX_CHUNK_SIZE_DEFAULT;
  SinglyLinkedList<MemoryChunk> storage;

 public:
  // All constructors are default: Rule of 0 in action
  explicit Allocator(size_t _max_chunk_size = MAX_CHUNK_SIZE_DEFAULT)
      : max_chunk_size(_max_chunk_size) {}

  Allocator(const Allocator &other)
      : max_chunk_size(other.max_chunk_size), storage(other.storage) {}

  Allocator &operator=(const Allocator &other) {
    if (this != &other) {
      max_chunk_size = other.max_chunk_size;
      storage = other.storage;
    }
    return *this;
  }

  ~Allocator() = default;

  template <class... Args>
  void construct(pointer p, Args &&... args) {
    new (p) T(args...);
  }

  void destroy(pointer p) {
    if (p) {
      p->~T();
    }
  }

  pointer allocate(size_type n) {
    size_t required_size = sizeof(value_type) * n;
    // try to find memory chunk with enough space
    auto *node = storage.GetHead();
    while (node != nullptr) {
      auto &chunk = node->value;
      if (chunk.size + required_size < chunk.capacity) {
        auto ret = reinterpret_cast<pointer>(chunk.data);
        chunk.size += required_size;
        return ret;
      }
      node = node->next;
    }
    // otherwise allocate new one
    if (required_size > max_chunk_size) {
      throw std::bad_array_new_length();
    }
    storage.PushFront(MemoryChunk());
    auto &new_chunk = storage.GetHead()->value;
    new_chunk = MemoryChunk(max_chunk_size);
    auto ret = reinterpret_cast<pointer>(new_chunk.data);
    new_chunk.size += required_size;
    return ret;
  }

  void deallocate(pointer p, size_type n) {
    // real deallocation is performed in destructor
  }

  // for tests
  [[nodiscard]] const SinglyLinkedList<MemoryChunk> &get_storage() const {
    return storage;
  }
};