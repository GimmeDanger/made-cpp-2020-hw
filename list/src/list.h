#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>

namespace task {

template <class T, class Allocator = std::allocator<T>>
class list {
  struct Node {
    T value = T();
    Node* prev = nullptr;
    Node* next = nullptr;
    template <class... Args>
    explicit Node(Node* prev_, Node* next_, Args&&... args)
        : value(std::forward<Args>(args)...), prev(prev_), next(next_) {}
  };

  class Iterator {
   public:
    using difference_type = ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::bidirectional_iterator_tag;

    explicit Iterator() = default;

    Iterator& operator++() {
      if (nullptr != curr) {
        prev = curr;
        curr = curr->next;
      }
      return *this;
    }

    Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    Iterator& operator--() {
      if (nullptr != prev) {
        curr = prev;
        prev = prev->prev;
      }
      return *this;
    }

    Iterator operator--(int) {
      Iterator tmp = *this;
      --(*this);
      return tmp;
    }

    reference operator*() const { return curr->value; }

    pointer operator->() const { return &(curr->value); }

    bool operator==(Iterator other) const {
      return prev == other.prev && curr == other.curr;
    }

    bool operator!=(Iterator other) const {
      return prev != other.prev && curr != other.curr;
    }

   protected:
    explicit Iterator(Node* prev_, Node* curr_) : prev(prev_), curr(curr_) {}

    Node* get_prev() { return prev; }
    Node* get_curr() { return curr; }

   private:
    Node* prev = nullptr;
    Node* curr = nullptr;
    friend class list<T, Allocator>;
  };

  class ConstIterator {
   public:
    using difference_type = ptrdiff_t;
    using value_type = const T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_category = std::bidirectional_iterator_tag;

    explicit ConstIterator() = default;

    ConstIterator(const Iterator& that) : prev(that.prev), curr(that.curr) {}

    ConstIterator& operator++() {
      if (nullptr != curr) {
        prev = curr;
        curr = curr->next;
      }
      return *this;
    }

    ConstIterator operator++(int) {
      ConstIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    ConstIterator& operator--() {
      if (nullptr != prev) {
        curr = prev;
        prev = prev->prev;
      }
      return *this;
    }

    ConstIterator operator--(int) {
      ConstIterator tmp = *this;
      --(*this);
      return tmp;
    }

    reference operator*() const { return curr->value; }

    pointer operator->() const { return &(curr->value); }

    bool operator==(ConstIterator other) const {
      return prev == other.prev && curr == other.curr;
    }

    bool operator!=(ConstIterator other) const {
      return prev != other.prev && curr != other.curr;
    }

   protected:
    explicit ConstIterator(Node* prev_, Node* curr_)
        : prev(prev_), curr(curr_) {}

    Node* get_prev() { return prev; }
    Node* get_curr() { return curr; }

   private:
    Node* prev = nullptr;
    Node* curr = nullptr;
    friend class list<T, Allocator>;
  };

 public:
  using value_type = T;
  using allocator_type = Allocator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename std::allocator_traits<allocator_type>::pointer;
  using const_pointer =
  typename std::allocator_traits<Allocator>::const_pointer;
  using iterator = Iterator;
  using const_iterator = ConstIterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 private:
  using NodeAllocator =
  typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;

 public:
  // construction methods
  explicit list(const Allocator& alloc_ = Allocator())
      : val_alloc(alloc_), node_alloc(NodeAllocator()) {}

  explicit list(size_type count, const T& value,
                const Allocator& alloc_ = Allocator())
      : val_alloc(alloc_), node_alloc(NodeAllocator()) {
    while (m_size != count) {
      push_back(value);
    }
  }

  explicit list(size_type count, const Allocator& alloc_ = Allocator())
      : val_alloc(alloc_), node_alloc(NodeAllocator()) {
    resize(count);
  }

  list(const list& other) {
    for (auto it = other.cbegin(); it != other.cend(); it++) {
      push_back(*it);
    }
  }

  list(list&& other) noexcept { this->swap(other); }

  list& operator=(const list& other) {
    if (this != &other) {
      clear();
      for (auto it = other.cbegin(); it != other.cend(); it++) {
        push_back(*it);
      }
    }
    return *this;
  }

  list& operator=(list&& other) noexcept {
    if (this != &other) {
      clear();
      this->swap(other);
    }
    return *this;
  }

  ~list() { clear(); }

  // Allocator
  Allocator get_allocator() const { return val_alloc; }

  // Getters
  reference front() { return *begin(); }
  const_reference front() const { return *cbegin(); }
  reference back() { return *std::prev(end()); }
  const_reference back() const { return *std::prev(cend()); }

  // Range utils
  iterator begin() { return iterator(nullptr, m_head); }
  iterator end() { return iterator(m_tail, nullptr); }
  const_iterator cbegin() const { return const_iterator(nullptr, m_head); }
  const_iterator cend() const { return const_iterator(m_tail, nullptr); }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  }

  // Capacity utils
  bool empty() const { return m_size == 0; }
  size_type size() const { return m_size; }
  size_type max_size() const {
    return std::numeric_limits<difference_type>::max();
  }

  // Modifiers
  void clear() {
    while (!empty()) {
      pop_back();
    }
  }

  // inserts value before pos
  iterator insert(const_iterator pos, const T& value) {
    auto* new_node = node_alloc.allocate(1);
    node_alloc.construct(new_node, pos.get_prev(), pos.get_curr(), value);
    return insert(pos, new_node);
  }

  iterator insert(const_iterator pos, T&& value) {
    auto* new_node = node_alloc.allocate(1);
    node_alloc.construct(new_node, pos.get_prev(), pos.get_curr(), std::move(value));
    return insert(pos, new_node);
  }

  iterator insert(const_iterator pos, size_type count, const T& value) {
    iterator it = Iterator(pos.get_prev(), pos.get_curr());
    for (size_type i = 0; i < count; i++) {
      it = insert(it, value);
    }
    return it;
  }

  // Erases the specified elements from the container.
  iterator erase(const_iterator pos) {
    auto* del_node = pos.get_curr();
    if (nullptr != del_node->prev) {
      del_node->prev->next = del_node->next;
      if (del_node == m_tail) {
        m_tail = del_node->prev;
      }
    }
    if (nullptr != del_node->next) {
      del_node->next->prev = del_node->prev;
      if (del_node == m_head) {
        m_head = del_node->next;
      }
    }
    if (--m_size == size_type(0)) {
      m_head = m_tail = nullptr;
    }
    auto ret = Iterator(del_node->prev, del_node->next);
    std::destroy_at(del_node);
    node_alloc.deallocate(del_node, 1);
    return ret;
  }

  iterator erase(const_iterator first, const_iterator last) {
    while (first != last) {
      first = erase(first);
    }
    return Iterator(first.get_prev(), first.get_curr());
  }

  void push_back(const T& value) { insert(cend(), value); }
  void push_back(T&& value) { insert(cend(), std::move(value)); }
  void pop_back() { erase(std::prev(end())); }

  void push_front(const T& value) { insert(cbegin(), value); }
  void push_front(T&& value) { insert(cbegin(), std::move(value)); }
  void pop_front() { erase(cbegin()); }

  template <class... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    auto* new_node = node_alloc.allocate(1);
    node_alloc.construct(new_node, pos.get_prev(), pos.get_curr(), std::forward<Args>(args)...);
    return insert(pos, new_node);
  }

  template <class... Args>
  void emplace_back(Args&&... args) {
    emplace(end(), std::forward<Args>(args)...);
  }

  template <class... Args>
  void emplace_front(Args&&... args) {
    emplace(begin(), std::forward<Args>(args)...);
  }

  // Resizes the container to contain count elements.
  void resize(size_t count) {
    while (m_size > count) {
      pop_back();
    }
    while (m_size < count) {
      emplace_back();
    }
  }

  void swap(list& other) noexcept {
    std::swap(m_size, other.m_size);
    std::swap(m_head, other.m_head);
    std::swap(m_tail, other.m_tail);
  }

  // merges two sorted lists
  void merge(list& other) {
    size_type new_size = m_size + other.m_size;
    m_head = merge_impl(m_head, other.m_head);
    fix_structure();
    assert(new_size = m_size);
    other.m_size = 0;
    other.m_head = nullptr;
    other.m_tail = nullptr;
  }

  // Transfers all elements from other into *this. The elements are inserted
  // before the element pointed to by pos
  void splice(const_iterator pos, list& other) {
    if (other.empty()) {
      return;
    }
    if (empty()) {
      *this = std::move(other);
    } else {
      auto* new_node_left = other.m_head;
      auto* new_node_right = other.m_tail;
      new_node_left->prev = pos.get_prev();
      new_node_right->next = pos.get_curr();
      if (nullptr != pos.get_prev()) {
        pos.get_prev()->next = new_node_left;
      }
      if (nullptr != pos.get_curr()) {
        pos.get_curr()->prev = new_node_right;
      }
      if (pos == cbegin()) {
        m_head = new_node_left;
      }
      if (pos == cend()) {
        m_tail = new_node_right;
      }
      m_size += other.m_size;
      other.m_size = 0;
      other.m_head = nullptr;
      other.m_tail = nullptr;
    }
  }

  // Removes all elements satisfying specific criteria
  void remove(const T value) {
    for (auto it = begin(); it != end();) {
      if (*it == value) {
        it = erase(it);
      } else {
        it++;
      }
    }
  }

  // reverses the order of the elements
  void reverse() {
    for (Node* curr = m_head; curr != nullptr;) {
      auto* prev = curr->prev;
      auto* next = curr->next;
      curr->prev = next;
      curr->next = prev;
      curr = next;
    }
    std::swap(m_head, m_tail);
  }

  // removes consecutive duplicate elements
  void unique() {
    for (auto it = begin(); it != end();) {
      auto range_end = std::find_if(it, end(), [it](auto& val) { return *it != val; });
      if (std::next(it) != range_end) {
        it = erase(std::next(it), range_end);
      } else {
        it++;
      }
    }
  }

  void sort() {
    if (m_head != m_tail) {
      size_type old_size = m_size;
      m_head = sort_impl(m_head);
      fix_structure();
      assert(old_size == m_size);
    }
  }

 private:
  void fix_structure() {
    if (nullptr != m_head) {
      m_head->prev = nullptr;
      auto* prev = m_head;
      auto* curr = m_head;
      m_size = 1;
      while (nullptr != curr && nullptr != curr->next) {
        curr = curr->next;
        curr->prev = prev;
        prev = curr;
        m_size++;
      }
      m_tail = curr;
    } else {
      m_size = 0;
      m_tail = m_head;
    }
  }

  static Node* split_impl(Node* head) {
    Node* mid_prev = nullptr;
    while (nullptr != head && nullptr != head->next) {
      if (mid_prev) {
        mid_prev = mid_prev->next;
      } else {
        mid_prev = head;
      }
      head = head->next->next;
    }
    Node* mid = mid_prev->next;
    mid_prev->next = nullptr;
    mid->prev = nullptr;
    return mid;
  }

  static Node* pop_front_node(Node** l) {
    if (nullptr == *l) {
      return nullptr;
    }
    Node* front = (*l);
    (*l) = (*l)->next;
    front->next = nullptr;
    return front;
  }

  static Node* pop_next_node(Node** l1, Node** l2) {
    if (nullptr == *l1) {
      return pop_front_node(l2);
    }
    if (nullptr == *l2) {
      return pop_front_node(l1);
    }
    Node** l = (*l1)->value <= (*l2)->value ? l1 : l2;
    return pop_front_node(l);
  }

  static Node* merge_impl(Node* l1, Node* l2) {
    Node* head = pop_next_node(&l1, &l2);
    Node* prev = head;
    while (nullptr != l1 || nullptr != l2) {
      Node* next = pop_next_node(&l1, &l2);
      prev->next = next;
      prev = prev->next;
      prev->next = nullptr;
    }

    return head;
  }

  static Node* sort_impl(Node* head) {
    if (nullptr == head || nullptr == head->next) {
      return head;
    }
    auto* mi = split_impl(head);
    Node* lo = sort_impl(head);
    Node* hi = sort_impl(mi);
    return merge_impl(lo, hi);
  }

  iterator insert(const_iterator pos, Node* new_node) {
    if (empty()) {
      m_head = m_tail = new_node;
    } else {
      if (nullptr != pos.get_prev()) {
        pos.get_prev()->next = new_node;
      }
      if (nullptr != pos.get_curr()) {
        pos.get_curr()->prev = new_node;
      }
      if (pos == cbegin()) {
        m_head = new_node;
      }
      if (pos == cend()) {
        m_tail = new_node;
      }
    }
    m_size++;
    return iterator(new_node->prev, new_node);
  }

 protected:
  Allocator val_alloc;
  NodeAllocator node_alloc;
  size_t m_size = 0;
  Node* m_head = nullptr;
  Node* m_tail = nullptr;
};

}  // namespace task

