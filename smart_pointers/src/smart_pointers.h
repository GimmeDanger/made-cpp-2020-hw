#pragma once

#include <algorithm> //< for std::swap

namespace task {

template <class T>
class UniquePtr {

  T *ptr = nullptr;

  void clear() {
    if (ptr) {
      delete ptr;
      ptr = nullptr;
    }
  }

 public:
  // default ctor and destructor
  explicit UniquePtr(T *ptr_ = nullptr) : ptr(ptr_) {}
  ~UniquePtr() { clear(); }

  // copy ctor and assignment operator
  UniquePtr(const UniquePtr<T> &that) = delete;
  UniquePtr<T> &operator=(const UniquePtr<T> &that) = delete;

  // move ctor and assignment operator
  UniquePtr(UniquePtr<T> &&that) noexcept {
    clear();
    ptr = that.release();
  }
  UniquePtr<T> &operator=(UniquePtr<T> &&that) noexcept {
    if (this != &that) {
      clear();
      ptr = that.release();
    }
    return *this;
  }

  // All sort of getters
  const T &operator*() const { return *ptr; }
  T &operator*() { return *ptr; }

  const T *operator->() const { return ptr; }
  T *operator->() { return ptr; }

  const T *get() const { return ptr; }
  T *get() { return ptr; }

  T *release() {
    T *ret = ptr;
    ptr = nullptr;
    return ret;
  }

  void reset(T *ptr_) {
    clear();
    ptr = ptr_;
  }

  void swap(UniquePtr<T> &that) {
    if (this != &that) {
      std::swap(ptr, that.ptr);
    }
  }
};

template <class T>
class WeakPtr;

template <class T>
class SharedPtr {
  class ControlBlock {
    T *ptr = nullptr;
    size_t shared_ptr_owners = 0;
    size_t weak_ptr_watchers = 0;
   public:
    explicit ControlBlock(T *ptr_ = nullptr) : ptr(ptr_) {}
    ~ControlBlock() { clear(); }

    T *get_ptr() { return ptr; }

    void decrease_owners() { shared_ptr_owners--; }
    void decrease_watchers() { weak_ptr_watchers--; }
    void increase_owners() { shared_ptr_owners++; }
    void increase_watchers() { weak_ptr_watchers++; }

    size_t get_shared_ptr_owners() const { return shared_ptr_owners; }
    size_t get_weak_ptr_watchers() const { return weak_ptr_watchers; }

    bool should_be_cleared() const { return shared_ptr_owners == size_t(0); }
    bool should_be_deallocated() const { return should_be_cleared() && weak_ptr_watchers == size_t(0); }

    void clear() {
      if (ptr) {
        delete ptr;
        ptr = nullptr;
      }
    }
  };
  using Ctrl = ControlBlock;

  T *ptr = nullptr;
  Ctrl *ctrl = nullptr;

  void clear() {
    if (ctrl) {
      ptr = nullptr;
      ctrl->decrease_owners();
      if (ctrl->should_be_cleared()) {
        ctrl->clear();
      }
      if (ctrl->should_be_deallocated()) {
        delete ctrl;
      }
    }
  }

  void init(Ctrl *ctrl_) {
    clear();
    if (ctrl_) {
      ctrl = ctrl_;
      ptr = ctrl->get_ptr();
      ctrl->increase_owners();
    }
  }

 public:

  // default ctor and destructor
  explicit SharedPtr(T *ptr_ = nullptr) {
    init(ptr_ ? new Ctrl(ptr_) : static_cast<Ctrl *>(nullptr));
  }

  SharedPtr(const WeakPtr<T> &that);

  ~SharedPtr() {
    clear();
  }

  // copy ctor and assignment operator
  SharedPtr(const SharedPtr<T> &that) {
    init(that.ctrl);
  };
  SharedPtr<T> &operator=(const SharedPtr<T> &that) {
    if (this != &that) {
      init(that.ctrl);
    }
    return *this;
  }

  // move ctor and assignment operator
  SharedPtr(SharedPtr<T> &&that) noexcept {
    if (that.ctrl) {
      this->swap(that);
    }
  }
  SharedPtr<T> &operator=(SharedPtr<T> &&that) noexcept {
    if (this != &that) {
      clear();
      this->swap(that);
    }
    return *this;
  }

  // All sort of getters
  const T &operator*() const { return *ptr; }
  T &operator*() { return *ptr; }

  const T *operator->() const { return ptr; }
  T *operator->() { return ptr; }

  const T *get() const { return ptr; }
  T *get() { return ptr; }

  size_t use_count() const {
    return ctrl ? ctrl->get_shared_ptr_owners() : size_t(0);
  }

  void reset(T *data_ = nullptr) {
    clear();
    SharedPtr<T> tmp(data_);
    this->swap(tmp);
  }

  void swap(SharedPtr<T> &that) {
    if (this != &that) {
      std::swap(ptr, that.ptr);
      std::swap(ctrl, that.ctrl);
    }
  }

  friend class WeakPtr<T>;
};

template <class T>
class WeakPtr {
  using Ctrl = typename SharedPtr<T>::ControlBlock;

  T *ptr = nullptr;
  Ctrl *ctrl = nullptr;

  void clear() {
    if (ctrl) {
      ptr = nullptr;
      ctrl->decrease_watchers();
      if (ctrl->should_be_deallocated()) {
        delete ctrl;
      }
    }
  }

  void init(Ctrl *ctrl_) {
    if (ctrl_) {
      ctrl = ctrl_;
      ptr = ctrl->get_ptr();
      ctrl->increase_watchers();
    }
  }

 public:

  // default ctor and destructor
  explicit WeakPtr() = default;

  WeakPtr(const SharedPtr<T> &that) {
    init(that.ctrl);
  }

  ~WeakPtr() {
    clear();
  }

  // copy ctor and assignment operator
  WeakPtr(const WeakPtr<T> &that) {
    init(that.ctrl);
  };
  WeakPtr<T> &operator=(const WeakPtr<T> &that) {
    if (this != &that) {
      init(that.ctrl);
    }
    return *this;
  }
  WeakPtr<T> &operator=(const SharedPtr<T> &that) {
    WeakPtr<T> tmp(that);
    this->swap(tmp);
    return *this;
  }

  // move ctor and assignment operator
  WeakPtr(WeakPtr<T> &&that) noexcept {
    if (that.ctrl) {
      this->swap(that);
    }
  }
  WeakPtr<T> &operator=(WeakPtr<T> &&that) noexcept {
    if (this != &that) {
      clear();
      this->swap(that);
    }
    return *this;
  }

  // All sort of getters
  bool expired() const {
    return !ctrl || ctrl->get_shared_ptr_owners() == size_t(0);
  }

  size_t use_count() const {
    return ctrl ? ctrl->get_shared_ptr_owners() : size_t(0);
  }

  SharedPtr<T> lock() const {
    return SharedPtr<T>(*this);
  }

  void reset(T *data_ = nullptr) {
    clear();
    WeakPtr<T> tmp(data_);
    this->swap(tmp);
  }

  void swap(WeakPtr<T> &that) {
    if (this != &that) {
      std::swap(ptr, that.ptr);
      std::swap(ctrl, that.ctrl);
    }
  }
  friend class SharedPtr<T>;
};

template<typename T>
SharedPtr<T>::SharedPtr(const WeakPtr<T> &that) {
  init(that.ctrl);
}

}  // namespace task
