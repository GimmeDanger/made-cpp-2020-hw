#include <forward_list>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "test_runner.h"
#include "src/allocator.h"

namespace TestSinglyLinkedList {
void RunTests();
}

namespace TestMemoryChunk {
void RunTests();
}

namespace TestAllocator {
void RunTests();
}

int main() {
  TestMemoryChunk::RunTests();
  TestSinglyLinkedList::RunTests();
  TestAllocator::RunTests();
  return 0;
}

namespace TestMemoryChunk {

std::vector<char> ToVector(const MemoryChunk& mc) {
  std::vector<char> res(mc.size);
  std::copy(res.begin(), res.end(), mc.data);
  return res;
}

void TestMemoryChunkConstructors() {
  {
    MemoryChunk a;
    ASSERT_EQUAL(0u, a.size);
    ASSERT_EQUAL(0u, a.capacity);
    ASSERT(a.data == nullptr);
  }
  {
    MemoryChunk a(50u);
    ASSERT_EQUAL(0u, a.size);
    ASSERT_EQUAL(50u, a.capacity);
    ASSERT(a.data != nullptr);
  }
  {
    MemoryChunk a(50u);
    a.size += 15;
    std::iota(a.data, a.data + a.size, 0);
    auto vec_a = ToVector(a);

    MemoryChunk b(100u);
    b.size += 25;
    std::iota(b.data, b.data + b.size, 0);

    a = b;

    ASSERT_EQUAL(25u, a.size);
    ASSERT_EQUAL(100u, a.capacity);
    ASSERT_EQUAL(ToVector(a), ToVector(b));
  }
  {
    MemoryChunk a(50u);
    a.size += 15;
    std::iota(a.data, a.data + a.size, 0);

    MemoryChunk b(a);
    ASSERT_EQUAL(15u, a.size);
    ASSERT_EQUAL(50u, a.capacity);
    ASSERT_EQUAL(ToVector(b), ToVector(a));
  }
}

void RunTests() {
  TestRunner tr;
  RUN_TEST(tr, TestMemoryChunkConstructors);
}
}  // namespace TestMemoryChunk

namespace TestSinglyLinkedList {

template <typename T>
std::vector<T> ToVector(const SinglyLinkedList<T>& list) {
  std::vector<T> result;
  for (auto node = list.GetHead(); node; node = node->next) {
    result.push_back(node->value);
  }
  return result;
}

void TestSinglyLinkedListConstructors() {
  SinglyLinkedList<std::string> a;
  ASSERT(ToVector(a).empty());

  a.PushFront("bac");
  a.PushFront("cab");
  ASSERT_EQUAL(2u, ToVector(a).size());

  SinglyLinkedList<std::string> b(a);
  ASSERT_EQUAL(2u, ToVector(b).size())
  ASSERT_EQUAL("cab", b.GetHead()->value);
  ASSERT_EQUAL("bac", b.GetHead()->next->value);

  SinglyLinkedList<std::string> c = b;
  ASSERT_EQUAL(2u, ToVector(c).size())
  ASSERT_EQUAL("cab", c.GetHead()->value);
  ASSERT_EQUAL("bac", c.GetHead()->next->value);

  SinglyLinkedList<std::string> d;
  d.PushFront("a");
  d = a;
  ASSERT_EQUAL(2u, ToVector(d).size())
  ASSERT_EQUAL("cab", d.GetHead()->value);
  ASSERT_EQUAL("bac", d.GetHead()->next->value);
}

void TestSinglyLinkedListPushFront() {
  SinglyLinkedList<int> list;

  list.PushFront(1);
  ASSERT_EQUAL(list.GetHead()->value, 1);
  list.PushFront(2);
  ASSERT_EQUAL(list.GetHead()->value, 2);
  list.PushFront(3);
  ASSERT_EQUAL(list.GetHead()->value, 3);

  const std::vector<int> expected = {3, 2, 1};
  ASSERT_EQUAL(ToVector(list), expected);
}

void TestSinglyLinkedListInsertAfter() {
  SinglyLinkedList<std::string> list;

  list.PushFront("a");
  auto head = list.GetHead();
  ASSERT(head);
  ASSERT_EQUAL(head->value, "a");

  list.InsertAfter(head, "b");
  const std::vector<std::string> expected1 = {"a", "b"};
  ASSERT_EQUAL(ToVector(list), expected1);

  list.InsertAfter(head, "c");
  const std::vector<std::string> expected2 = {"a", "c", "b"};
  ASSERT_EQUAL(ToVector(list), expected2);
}

void TestSinglyLinkedListRemoveAfter() {
  SinglyLinkedList<int> list;
  for (int i = 1; i <= 5; ++i) {
    list.PushFront(i);
  }

  const std::vector<int> expected = {5, 4, 3, 2, 1};
  ASSERT_EQUAL(ToVector(list), expected);

  auto next_to_head = list.GetHead()->next;
  list.RemoveAfter(next_to_head);
  list.RemoveAfter(next_to_head);

  const std::vector<int> expected1 = {5, 4, 1};
  ASSERT_EQUAL(ToVector(list), expected1);

  while (list.GetHead()->next) {
    list.RemoveAfter(list.GetHead());
  }
  ASSERT_EQUAL(list.GetHead()->value, 5);
}

void TestSinglyLinkedListPopFront() {
  SinglyLinkedList<int> list;

  for (int i = 1; i <= 5; ++i) {
    list.PushFront(i);
  }
  for (int i = 1; i <= 5; ++i) {
    list.PopFront();
  }
  ASSERT(list.GetHead() == nullptr);
}

void RunTests() {
  TestRunner tr;
  RUN_TEST(tr, TestSinglyLinkedListConstructors);
  RUN_TEST(tr, TestSinglyLinkedListPushFront);
  RUN_TEST(tr, TestSinglyLinkedListInsertAfter);
  RUN_TEST(tr, TestSinglyLinkedListRemoveAfter);
  RUN_TEST(tr, TestSinglyLinkedListPopFront);
}
}  // namespace TestSinglyLinkedList

namespace TestAllocator {

template <typename T>
std::vector<char> ToVector(const Allocator<T>& all) {
  std::vector<char> res;
  for (auto& ch : TestSinglyLinkedList::ToVector(all.get_storage())) {
    for (size_t i = 0; i < ch.size; i++) res.push_back(ch.data[i]);
  }
  return res;
}

void TestAllocatorBasic() {
  // cpp-reference example
  {
    Allocator<int> a1;        // default allocator for ints
    int* a = a1.allocate(1);  // space for one int
    a1.construct(a, 7);       // construct the int
    ASSERT_EQUAL(a[0], 7);
    a1.deallocate(a, 1);  // deallocate space for one int

    // default allocator for strings
    Allocator<std::string> a2;

    // same, but obtained by rebinding from the type of a1
    decltype(a1)::rebind<std::string>::other a2_1;

    // same, but obtained by rebinding from the type of a1 via allocator_traits
    std::allocator_traits<decltype(a1)>::rebind_alloc<std::string> a2_2;
    std::string* s = a2.allocate(2);  // space for 2 strings

    a2.construct(s, "foo");
    a2.construct(s + 1, "bar");
    ASSERT_EQUAL(s[0], "foo");
    ASSERT_EQUAL(s[1], "bar");

    a2.destroy(s);
    a2.destroy(s + 1);
    a2.deallocate(s, 2);
  }
}

void TestAllocatorConstructors() {
  {
    size_t chunk_len = sizeof(std::string);
    Allocator<std::string> all1(chunk_len + 10);
    ASSERT_EQUAL(0u, ToVector(all1).size());  //< single chunk allocated

    std::string* data1 = all1.allocate(1);
    ASSERT_EQUAL(chunk_len, ToVector(all1).size());  //< single chunk allocated
    all1.construct(&data1[0], "bac");
    ASSERT_EQUAL(data1[0], "bac");

    std::string* data2 = all1.allocate(1);
    ASSERT_EQUAL(2 * chunk_len,
                 ToVector(all1).size());  //< single chunk allocated
    all1.construct(&data2[0], "cab");
    ASSERT_EQUAL(data2[0], "cab");

    auto all1_storage_before_constr = ToVector(all1);

    // copy ctor
    Allocator<std::string> all2(all1);
    ASSERT_EQUAL(2 * chunk_len, ToVector(all1).size());
    ASSERT_EQUAL(2 * chunk_len, ToVector(all2).size());

    ASSERT_EQUAL(ToVector(all1), ToVector(all2));
    ASSERT_EQUAL(ToVector(all1), all1_storage_before_constr);

    // copy assignment
    Allocator<std::string> all3;
    all3 = all1;
    ASSERT_EQUAL(2 * chunk_len, ToVector(all1).size());
    ASSERT_EQUAL(2 * chunk_len, ToVector(all3).size());
    ASSERT_EQUAL(ToVector(all1), ToVector(all3));
    ASSERT_EQUAL(ToVector(all1), all1_storage_before_constr);
  }
}

void TestAllocatorMethods() {
  Allocator<double> all(25);
  ASSERT_EQUAL(0u, ToVector(all).size());  //< single chunk allocated

  double* a = all.allocate(2);  // space for two doubles
  ASSERT_EQUAL(2 * sizeof(double), ToVector(all).size());

  all.construct(&a[0], 42.);  // construct the double
  all.construct(&a[1], 43.);  // construct the double
  ASSERT_EQUAL(2 * sizeof(double), ToVector(all).size());

  double* b = all.allocate(2);  // space for two other doubles
  ASSERT_EQUAL(4 * sizeof(double),
               ToVector(all).size());  //< single chunk allocated

  all.construct(&b[0], 44.);  // construct the double
  all.construct(&b[1], 45.);  // construct the double
  ASSERT_EQUAL(4 * sizeof(double),
               ToVector(all).size());  //< single chunk allocated

  try {
    double* c = all.allocate(
        5);  // space for five double (5 * 8 = 30 > capacity of single chunk)
    all.deallocate(c, 5);
  } catch (std::exception& ex) {
    ASSERT_EQUAL(ex.what(), std::string("std::bad_array_new_length"));
  }
}

void TestAllocatorWithStlContainers() {
  {
    std::vector<std::string, Allocator<std::string>> vec;
    auto expected = std::vector{"bac1", "cab2", "bac3", "cab4", "bac5"};
    std::copy(expected.begin(), expected.end(), std::back_inserter(vec));
    for (size_t i = 0; i < vec.size(); i++) {
      ASSERT_EQUAL(expected[i], vec[i]);
    }
  }
}

void RunTests() {
  TestRunner tr;
  RUN_TEST(tr, TestAllocatorBasic);
  RUN_TEST(tr, TestAllocatorMethods);
  RUN_TEST(tr, TestAllocatorConstructors);
  RUN_TEST(tr, TestAllocatorWithStlContainers);
}
}  // namespace TestAllocator