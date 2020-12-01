#include "compose.h"
#include "function.h"
#include "test_runner.h"

#include <functional>
#include <sstream>
#include <vector>

// Instances of std::function can store, copy, and invoke any CopyConstructible
// Callable target
// -- functions,
// -- lambda expressions,
// -- bind expressions,
// -- or other function objects, as well as
// -- -- pointers to member functions
// -- -- and pointers to data members.

namespace testing {

template <class F, class G>
void test_base(F f, G g) {
  using FunctionT = std::string(int);
  // Constructor / assignment by functor
  {
    ::function<FunctionT> func(f);
    std::function<FunctionT> std_func(f);
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func(i), std_func(i));
    }
    func = g;
    std_func = g;
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func(i), std_func(i));
    }
    func = {g};
    std_func = {g};
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func(i), std_func(i));
    }
    auto func_other = func;
    func.swap(func_other);
    auto std_func_other = std_func;
    std_func.swap(std_func_other);
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func(i), std_func(i));
    }
  }
  // Assignment operator by functor (initializer list)
  {
    ::function<FunctionT> func = f;
    std::function<FunctionT> std_func;
    std_func = f;
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func(i), std_func(i));
    }
  }
  // Copy constructor
  {
    ::function<FunctionT> func(f);
    ::function<FunctionT> func_other(func);
    std::function<FunctionT> std_func(f);
    std::function<FunctionT> std_func_other(std_func);
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func_other(i), std_func_other(i));
    }
  }
  // Assignment copy operator
  {
    ::function<FunctionT> func(f);
    ::function<FunctionT> func_func(g);
    ::function<FunctionT> func_other;
    func_other = func_func;
    std::function<FunctionT> std_func(f);
    std::function<FunctionT> std_func_func(g);
    std::function<FunctionT> std_func_other;
    std_func_other = std_func_func;
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func_other(i), std_func_other(i));
    }
  }
  // Move constructor
  {
    ::function<FunctionT> func((::function<FunctionT>(f)));
    std::function<FunctionT> std_func((std::function<FunctionT>(f)));
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func(i), std_func(i));
    }
  }
  // Assignment move operator
  {
    ::function<FunctionT> func(f);
    ::function<FunctionT> func_other;
    func_other = ::function<FunctionT>(g);
    std::function<FunctionT> std_func(f);
    std::function<FunctionT> std_func_other;
    std_func_other = std::function<FunctionT>(g);
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func_other(i), std_func_other(i));
    }
  }
  // bool, ==, != operators
  {
    ::function<FunctionT> func;
    ASSERT(func == nullptr);
    ASSERT(nullptr == func);
    ASSERT(!static_cast<bool>(func));
    func = f;
    ASSERT(func != nullptr);
    ASSERT(nullptr != func);
    ASSERT(static_cast<bool>(func));
  }
}

enum class type_t {
  obj,
  ref,
  ptr,
};

template <class T>
struct RemoveAll
{
  using type = typename std::conditional<
      std::is_reference<T>::value,
      typename std::remove_reference<T>::type,
      typename std::conditional<
          std::is_pointer<T>::value,
          typename std::remove_pointer<T>::type,
          T>::type
  >::type;
};

template <class ClassF, class ClassG, class MemberF, class MemberG>
void test_member_base(MemberF f, MemberG g) {
  typename RemoveAll<ClassF>::type obj_f;
  typename RemoveAll<ClassG>::type obj_g;
  using TypeF = std::string(ClassF, int);
  using TypeG = std::string(ClassG, int);
  // Constructor / assignment by functor
  {
    ::function<TypeF> func(f);
    std::function<TypeF> std_func(f);
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func(obj_f, i), std_func(obj_f, i));
    }
    ::function<TypeG> func_other;
    func_other = g;
    std::function<TypeG> std_func_other;
    std_func_other = g;
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func_other(obj_g, i), std_func_other(obj_g, i));
    }
  }
  // Copy constructor
  {
    ::function<TypeF> func(f);
    ::function<TypeF> func_other(func);
    std::function<TypeF> std_func(f);
    std::function<TypeF> std_func_other(std_func);
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func_other(obj_f, i), std_func_other(obj_f, i));
    }
  }
  // Assignment copy operator
  {
    ::function<TypeG> func_func(g);
    ::function<TypeG> func_other;
    func_other = func_func;
    std::function<TypeG> std_func_func(g);
    std::function<TypeG> std_func_other;
    std_func_other = std_func_func;
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func_other(obj_g, i), std_func_other(obj_g, i));
    }
  }
  // Move constructor
  {
    ::function<TypeF> func((::function<TypeF>(f)));
    std::function<TypeF> std_func((std::function<TypeF>(f)));
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func(obj_f, i), std_func(obj_f, i));
    }
  }
  // Assignment move operator
  {
    ::function<TypeF> func(f);
    ::function<TypeF> func_other;
    func_other = ::function<TypeF>(f);
    std::function<TypeF> std_func(f);
    std::function<TypeF> std_func_other;
    std_func_other = std::function<TypeF>(f);
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func_other(obj_f, i), std_func_other(obj_f, i));
    }
  }
  // bool, ==, != operators
  {
    ::function<TypeF> func;
    ASSERT(func == nullptr);
    ASSERT(nullptr == func);
    ASSERT(!static_cast<bool>(func));
    func = f;
    ASSERT(func != nullptr);
    ASSERT(nullptr != func);
    ASSERT(static_cast<bool>(func));
  }
}

template <class ClassF, class ClassG, class MemberF, class MemberG>
void test_ptr_member_base(MemberF f, MemberG g) {
  typename RemoveAll<ClassF>::type obj_f;
  typename RemoveAll<ClassG>::type obj_g;
  using TypeF = std::string(ClassF, int);
  using TypeG = std::string(ClassG, int);
  // Constructor / assignment by functor
  {
    ::function<TypeF> func(f);
    std::function<TypeF> std_func(f);
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func(&obj_f, i), std_func(&obj_f, i));
    }
    ::function<TypeG> func_other;
    func_other = g;
    std::function<TypeG> std_func_other;
    std_func_other = g;
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func_other(&obj_g, i), std_func_other(&obj_g, i));
    }
  }
  // Copy constructor
  {
    ::function<TypeF> func(f);
    ::function<TypeF> func_other(func);
    std::function<TypeF> std_func(f);
    std::function<TypeF> std_func_other(std_func);
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func_other(&obj_f, i), std_func_other(&obj_f, i));
    }
  }
  // Assignment copy operator
  {
    ::function<TypeG> func_func(g);
    ::function<TypeG> func_other;
    func_other = func_func;
    std::function<TypeG> std_func_func(g);
    std::function<TypeG> std_func_other;
    std_func_other = std_func_func;
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func_other(&obj_g, i), std_func_other(&obj_g, i));
    }
  }
  // Move constructor
  {
    ::function<TypeF> func((::function<TypeF>(f)));
    std::function<TypeF> std_func((std::function<TypeF>(f)));
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func(&obj_f, i), std_func(&obj_f, i));
    }
  }
  // Assignment move operator
  {
    ::function<TypeF> func(f);
    ::function<TypeF> func_other;
    func_other = ::function<TypeF>(f);
    std::function<TypeF> std_func(f);
    std::function<TypeF> std_func_other;
    std_func_other = std::function<TypeF>(f);
    for (int i = 0; i < 10; i++) {
      ASSERT_EQUAL(func_other(&obj_f, i), std_func_other(&obj_f, i));
    }
  }
  // bool, ==, != operators
  {
    ::function<TypeF> func;
    ASSERT(func == nullptr);
    ASSERT(nullptr == func);
    ASSERT(!static_cast<bool>(func));
    func = f;
    ASSERT(func != nullptr);
    ASSERT(nullptr != func);
    ASSERT(static_cast<bool>(func));
  }
}

namespace empty {
void test() {
  ::function<void(double)> f_empty;
  f_empty = std::nullptr_t();
  // f_empty(50.);
  ::function<std::vector<int>(std::string)> f_nullptr(std::nullptr_t);
  // f_nullptr("hello");
}
}  // namespace empty

namespace lambda {
void test() {
  auto f = [](int i) -> std::string { return std::to_string(i) + "f"; };
  auto g = [](int i) -> std::string { return std::to_string(i) + "g"; };
  test_base(f, g);
}
}  // namespace lambda

namespace free_function {
std::string f(int i) { return std::to_string(i) + "f"; }
std::string g(int i) { return std::to_string(i) + "g"; }
void test() { test_base(f, g); }
}  // namespace free_function

namespace std_function {
void test() {
  auto f = [](int i) -> std::string { return std::to_string(i) + "f"; };
  auto g = [](int i) -> std::string { return std::to_string(i) + "g"; };
  test_base(std::function<std::string(int)>(f),
            std::function<std::string(int)>(g));
}
}  // namespace std_function

namespace member_function {
struct F {
  F() {}
  std::string print(int i) { return std::to_string(i) + "f"; }
  std::string print_const(int i) const { return std::to_string(i) + "f"; }
};
struct G {
  G() {}
  std::string print(int i) { return std::to_string(i) + "g"; }
  std::string print_const(int i) const { return std::to_string(i) + "g"; }
};
void test() {
  test_member_base<F, G>(&F::print, &G::print);
  test_member_base<const F, const G>(&F::print, &G::print);
  test_member_base<F, G>(&F::print_const, &G::print_const);
  test_member_base<const F, const G>(&F::print_const, &G::print_const);

  test_member_base<F&, G&>(&F::print, &G::print);
  // test_member_base<F&, G&>(&F::print_const, &G::print_const); //< won't compile
  test_member_base<const F&, const G&>(&F::print_const, &G::print_const);

  // не успел объединить test_member_base и test_ptr_member_base в один метод,
  // поэтому пришлось копипастнуть в рамках тестов:

  test_ptr_member_base<F*, G*>(&F::print, &G::print);
  // test_ptr_member_base<const F*, const G*>(&F::print, &G::print); //< won't compile
  test_ptr_member_base<const F*, const G*>(&F::print_const, &G::print_const);
}
}  // namespace member_function

namespace std_bind {
std::string free_func(int i, std::string str) {
  return std::to_string(i) + str;
}
void test() {
  auto f = std::bind(free_func, std::placeholders::_1, "f");
  auto g = std::bind(free_func, std::placeholders::_1, "g");
  test_base(f, g);
}
}  // namespace std_bind

namespace function_object {
struct F {
  std::string operator()(int i) { return std::to_string(i) + "f"; }
};
struct G {
  std::string operator()(int i) { return std::to_string(i) + "g"; }
};
void test() { test_base(F(), G()); }
}  // namespace function_object

}  // namespace testing

int main() {
  TestRunner tr;
  tr.RunTest(testing::empty::test, "test empty");
  tr.RunTest(testing::lambda::test, "test lambdas");
  tr.RunTest(testing::free_function::test, "test free function");
  tr.RunTest(testing::std_function::test, "test std function");
  tr.RunTest(testing::std_bind::test, "test std bind");
  tr.RunTest(testing::function_object::test, "test functional object");
  tr.RunTest(testing::member_function::test, "test member function");
  return 0;
}
