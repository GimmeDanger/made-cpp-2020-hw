#pragma once

#include <memory>
#include <type_traits>

template <class>
class function;

template <class ReturnType, class... Args>
class function<ReturnType(Args...)> {
  class callable_base {
   public:
    callable_base() = default;
    callable_base(const callable_base&) = delete;
    callable_base& operator=(const callable_base&) = delete;
    virtual ~callable_base() = default;
    virtual ReturnType operator()(Args... args) = 0;
    virtual std::unique_ptr<callable_base> clone() = 0;
  };

  template <typename Functor>
  class callable : public callable_base {
    Functor m_func;

   public:
    explicit callable(Functor func) : m_func(func) {}
    virtual ReturnType operator()(Args... args) {
      return m_func(std::forward<Args>(args)...);
    }
    virtual std::unique_ptr<callable_base> clone() {
      return std::make_unique<callable>(m_func);
    }
  };

  template <typename FunctionT, typename ClassT, typename ... OtherArgs>
  class callable_member : public callable_base {
    using signature_t = FunctionT ClassT::*;
    signature_t m_func;

   public:
    explicit callable_member(signature_t func) : m_func(func) {}
    virtual ReturnType operator()(ClassT obj, OtherArgs... args) {
      return (obj.*m_func)(args...);
    }
    virtual std::unique_ptr<callable_base> clone() {
      return std::make_unique<callable_member>(m_func);
    }
  };

  template <typename FunctionT, typename ClassTRef, typename ... OtherArgs>
  class callable_member<FunctionT, ClassTRef&, OtherArgs...> : public callable_base {
    using ClassT = typename std::remove_reference<ClassTRef>::type;
    using signature_t = FunctionT ClassT::*;
    signature_t m_func;

   public:
    explicit callable_member(signature_t func) : m_func(func) {}
    virtual ReturnType operator()(ClassT& obj, OtherArgs... args) {
      return (obj.*m_func)(args...);
    }
    virtual std::unique_ptr<callable_base> clone() {
      return std::make_unique<callable_member>(m_func);
    }
  };

  template <typename FunctionT, typename ClassTPtr, typename ... OtherArgs>
  class callable_member<FunctionT, ClassTPtr*, OtherArgs...> : public callable_base {
    using ClassT = typename std::remove_pointer<ClassTPtr>::type;
    using signature_t = FunctionT ClassT::*;
    signature_t m_func;

   public:
    explicit callable_member(signature_t func) : m_func(func) {}
    virtual ReturnType operator()(ClassT* obj, OtherArgs... args) {
      return ((*obj).*m_func)(args...);
    }
    virtual std::unique_ptr<callable_base> clone() {
      return std::make_unique<callable_member>(m_func);
    }
  };

 public:
  function() = default;

  explicit function(std::nullptr_t) noexcept {}

  function(const function& other) : m_invoker(other.m_invoker->clone()) {}

  function(function&& other) noexcept : m_invoker(std::move(other.m_invoker)) {}

  function& operator=(std::nullptr_t) noexcept {
    m_invoker.reset();
    return *this;
  }

  function& operator=(const function& other) {
    if (this != &other) {
      m_invoker = other.m_invoker->clone();
    }
    return *this;
  }

  function& operator=(function&& other) noexcept {
    if (this != &other) {
      m_invoker = std::move(other.m_invoker);
    }
    return *this;
  }

  template <typename Functor>
  function(Functor func)
      : m_invoker(std::make_unique<callable<Functor>>(func)) {}

  template <typename FunctionT, typename ClassT>
  function(FunctionT ClassT::*f)
      : m_invoker(std::make_unique<callable_member<FunctionT, Args...>>(f)) {}

  template <typename FunctionT, typename ClassTRef>
  function(FunctionT std::remove_reference<ClassTRef>::type::*f)
      : m_invoker(std::make_unique<callable_member<FunctionT, Args...>>(f)) {}

  template <typename FunctionT, typename ClassTPtr>
  function(FunctionT std::remove_pointer<ClassTPtr>::type::*f)
      : m_invoker(std::make_unique<callable_member<FunctionT, Args...>>(f)) {}

  template <typename Functor>
  function& operator=(Functor&& f) {
    m_invoker = std::make_unique<callable<Functor>>(f);
    return *this;
  }

  template <typename FunctionT, typename ClassT>
  function& operator=(FunctionT ClassT::*f) {
    m_invoker = std::make_unique<callable_member<FunctionT, Args...>>(f);
    return *this;
  }

  template <typename FunctionT, typename ClassTRef>
  function& operator=(FunctionT std::remove_reference<ClassTRef>::type::*f) {
    m_invoker = std::make_unique<callable_member<FunctionT, Args...>>(f);
    return *this;
  }

  template <typename FunctionT, typename ClassTPtr>
  function& operator=(FunctionT std::remove_pointer<ClassTPtr>::type::*f) {
    m_invoker = std::make_unique<callable_member<FunctionT, Args...>>(f);
    return *this;
  }

  void swap(function &other) noexcept {
    std::swap(m_invoker, other.m_invoker);
  }

  // invokes the target
  ReturnType operator()(Args... args) {
    return (*m_invoker)(std::forward<Args>(args)...);
  }

  explicit operator bool() const noexcept {
    return m_invoker != nullptr;
  }

 private:
  using invoker_type = std::unique_ptr<callable_base>;
  invoker_type m_invoker;
};

template <class ReturnType, class... Args>
bool operator==(const function<ReturnType(Args...)>& f,
                std::nullptr_t) noexcept {
  return !static_cast<bool>(f);
}

template <class ReturnType, class... Args>
bool operator!=(const function<ReturnType(Args...)>& f,
                std::nullptr_t) noexcept {
  return static_cast<bool>(f);
}

template <class ReturnType, class... Args>
bool operator==(std::nullptr_t,
                const function<ReturnType(Args...)>& f) noexcept {
  return f == nullptr;
}

template <class ReturnType, class... Args>
bool operator!=(std::nullptr_t,
                const function<ReturnType(Args...)>& f) noexcept {
  return f != nullptr;
}