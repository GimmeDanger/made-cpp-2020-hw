#pragma once

template <typename T>
class SinglyLinkedList {
 public:
  struct Node {
    T value;
    Node* next = nullptr;
  };

 private:
  Node* head = nullptr;

  // free all resources
  void Clear();

 public:
  SinglyLinkedList() = default;
  SinglyLinkedList(const SinglyLinkedList<T>& other);
  SinglyLinkedList<T>& operator=(const SinglyLinkedList<T>& other);
  SinglyLinkedList(SinglyLinkedList<T>&& other) noexcept = delete;
  SinglyLinkedList<T>& operator=(SinglyLinkedList<T>&& other) noexcept = delete;
  ~SinglyLinkedList();

  // Modification methods
  void PushFront(const T& value);
  void InsertAfter(Node* node, const T& value);
  void RemoveAfter(Node* node);
  void PopFront();

  // Getters
  Node* GetHead() { return head; }
  [[nodiscard]] const Node* GetHead() const { return head; }
};

template <typename T>
SinglyLinkedList<T>::SinglyLinkedList(const SinglyLinkedList<T>& other) {
  Node* tail = head;
  auto* curr = other.GetHead();
  while (curr != nullptr) {
    InsertAfter(tail, curr->value);
    tail = tail ? tail->next : head;
    curr = curr->next;
  }
}

template <typename T>
SinglyLinkedList<T>& SinglyLinkedList<T>::operator=(
    const SinglyLinkedList<T>& other) {
  if (this != &other) {
    Clear();
    Node* tail = head;
    auto* curr = other.GetHead();
    while (curr != nullptr) {
      InsertAfter(tail, curr->value);
      tail = tail ? tail->next : head;
      curr = curr->next;
    }
  }
  return *this;
}

template <typename T>
void SinglyLinkedList<T>::Clear() {
  while (head != nullptr) {
    PopFront();
  }
}

template <typename T>
SinglyLinkedList<T>::~SinglyLinkedList() {
  Clear();
}

template <typename T>
void SinglyLinkedList<T>::PushFront(const T& value) {
  InsertAfter(nullptr, value);
}

template <typename T>
void SinglyLinkedList<T>::InsertAfter(Node* node, const T& value) {
  Node** next = node ? &(node->next) : &head;
  *next = new Node{value, *next};
}

template <typename T>
void SinglyLinkedList<T>::PopFront() {
  RemoveAfter(nullptr);
}

template <typename T>
void SinglyLinkedList<T>::RemoveAfter(Node* node) {
  Node** next = node ? &(node->next) : &head;
  if (*next) {
    Node* tmp = (*next)->next;
    delete *next;
    *next = tmp;
  }
}