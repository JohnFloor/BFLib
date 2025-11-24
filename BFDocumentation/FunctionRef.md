# `BF::FunctionRef`

`BF::FunctionRef` is a type-erased function view. It views/refers to, rather than contains (like `std::function`) a callable. A similar function view&mdash;`std::function_ref`&mdash;is proposed into the C++ Standard Library.


## Distinctive features


### Strict signature matching

In case of [standard function wrappers](#std-wrappers) you can initialize the wrapper with a callable of different, but compatible signature. If the wrapper's `operator()` is called, it converts the parameter and return types.

`BF::FunctionRef` accepts only exactly matching signatures. This includes the return and parameter types as well.

```c++
void Foo1(int) {}
void Foo2(const int&) {}

std::function<void (int)> f1 = &Foo1;           // OK
std::function<void (int)> f2 = &Foo2;           // OK

BF::FunctionRef<void (int)> g1 = &Foo1;         // OK
BF::FunctionRef<void (int)> g2 = &Foo2;         // error: different signatures
```

This makes `BF::FunctionRef` suitable for overloading on convertible callback signatures. As an example, you can make a container class with two `Enumerate()` methods differing only in the callback's return type.

```c++
struct MyContainer {
    void Enumerate(BF::FunctionRef<void ()>);   // enumerate all items unconditionally
    void Enumerate(BF::FunctionRef<bool ()>);   // stop enumeration, if client returns false
};

int main() {
    MyContainer cont;
    cont.Enumerate([] {});                      // calls 1st candidate
    cont.Enumerate([] { return false; });       // calls 2nd candidate
}
```

The above wouldn't work with a standard function wrapper or view, because the 2<sup>nd</sup> call would be ambiguous.

You can also have a non-`const` and `const` overload of the same `Enumerate()` method. Of course, they have to differ in the callable's signature too. Example:

```c++
template <class Type>
struct MyContainer {
    void Enumerate(BF::FunctionRef<void (Type&)> f);
    void Enumerate(BF::FunctionRef<void (const Type&)> f) const;
};

int main() {
    MyContainer<double> cont;

    cont.Enumerate(BF::FunctionRef<void (double&)>{});          // calls 1st candidate
    cont.Enumerate(BF::FunctionRef<void (const double&)>{});    // calls 2nd candidate

    cont.Enumerate([] (double&) {});                            // calls 1st candidate
    cont.Enumerate([] (const double&) {});                      // calls 2nd candidate
}
```

The above wouldn't work with a standard function wrapper or view, because the 2<sup>nd</sup> call would be ambiguous, and the 4<sup>th</sup> would surprisingly call the 1<sup>st</sup> candidate.


### Const-correctness

`BF::FunctionRef` forwards the `const` qualifier of its signature template parameter to the call operator. E.g.,
- `BF::FunctionRef<void()>::operator()` is not `const`, but
- `BF::FunctionRef<void() const>::operator()` is a `const` method.

Const-correctness of standard function wrappers and views:
- The function wrapper `std::function` is not const-correct. `operator()` is always `const`, thus it can modify the contained object even if the containing `std::function` is `const`. This was corrected in the later `std::move_only_function` and `std::copyable_function`.
- The function wrappers `std::move_only_function` and `std::copyable_function` are const-correct. Their template parameter supports signatures of the form `Ret (Pars...)` `const`<sub>op</sub> `(&|&&)`<sub>op</sub> `noexcept`<sub>op</sub> and they forward all qualifiers/specifiers to `operator()`.
- Whether the function view `std::function_ref` is const-correct, is arguable. Its template parameter supports signatures of the form `Ret (Pars...)` `const`<sub>op</sub> `noexcept`<sub>op</sub>. It forwards the `noexcept` specifier, but not the `const` qualifier, to its `operator()`, which in turn is always `const`. This means, calling the `operator()` of a `const std::function_ref` can alter program state. This, however, seems to be intentional. It mimics the way an `int*` works. In C++ an `int*` member variable in a `const` object is `int* const` and not `const int* const`.

`BF::FunctionRef` chose to forward the `const` qualifier from the template parameter to `operator()`. The author thinks, if `BF::FunctionRef<void ()> f` is a member variable of a `const` object, in 99.9% of the cases the programmer's intention is, that `f()` should not modify the pointee. In the remaining 0.1% one can still apply a `const_cast`. See the [`ConstCast` method](#constcast-method).

The inevitable corollary of const-correct function wrappers and views is that they usually cannot be `const` `&` in function parameters. Consider:

```c++
void Foo(const BF::FunctionRef<void ()>& f)
{
    f();    // error: 'f' is const, but the called 'operator()' is not
}
```

As a best practice, we pass these by value:

```c++
void Foo(BF::FunctionRef<void ()> f)
{
    f();    // OK
}
```

They can be `const` `&` only if `Signature` contains `const`.


### Detailed diagnostic messages with `static_assert`

In most scenarios, where the equivalent standard solution would give an obscure error message, `BF::FunctionRef` gives a `static_assert`, that directly describes the error situation.

The most helpful `static_assert`'s are the ones that are issued, when overloading is involved: `BF::FunctionRef` is the parameter of an overloaded function, or the initializer is an overloaded function or a functor with overloaded `operator()`.

There are also `static_assert`'s in the form `"ILE: ..."` (Internal Library Error). These cannot be triggered by clients, unless there is a bug in the library.


## Initialization

`BF::FunctionRef`'s template parameter&mdash;the `Signature`&mdash;should be in the form `Ret (Pars...)` `const`<sub>op</sub> `noexcept`<sub>op</sub>.

The `const` and `noexcept` in `Signature` is forwarded to `BF::FunctionRef::operator()`:
- `Signature` contains `const` <=> `BF::FunctionRef::operator()` is `const`.
- `Signature` contains `noexcept` <=> `BF::FunctionRef::operator()` is `noexcept`.

**Definition**. Let `FWrap` be a function wrapper/view, and `f` an expression denoting a (possibly overloaded) function or a function object. We say, that *`FWrap` accepts `f` on its public interface*, if `FWrap`'s ctor. and `operator=` can be chosen unambiguously by overload resolution, when `FWrap` is initialized/assigned from `f`. This means that the parameter types of the ctor. and `operator=` accept the type and value category of `f`, and the type constraints (if any) are also satisfied. (We assume, that `FWrap`'s ctors. have the same parameter types and constraints as its `operator=`'s.)

Note, that further constraints can be checked after overload resolution chose the ctor./`operator=`. These can be checked in the implementation of the chosen function in `static_assert`'s. To make `BF::FunctionRef` easy to use, it is crucial to choose wisely what to check on the public interface, and what in the implementation.


### Initializing from a function

`BF::FunctionRef` will accept a function on its public interface, if it returns `Ret` and has parameters `Pars...`. Only this property is considered during overload resolution. The rest is checked later by `static_assert`'s. Example:

```c++
void Foo();                                     // not noexcept

void Overloaded(BF::FunctionRef<void () noexcept>);
void Overloaded(BF::FunctionRef<void (int) noexcept>);

int main()
{
    Overloaded(&Foo);                           // static_assert: 'Foo' is not 'noexcept'
}
```

The call in `main` successfully selects the 1<sup>st</sup> candidate. The `static_assert` is issued afterwards. In a properly written compiler, the template instantiation context will mention the declaration of the selected `Overloaded` candidate and the call site. This is easier to debug, than the general *no overloaded function could convert all the argument types*, as would be the case with the standard function wrappers, which check everything on their public interface.

The following properties are checked by `static_assert`'s:

- If `Signature` contains `const`, it is ignored. Example:
  ```c++
  void Foo();

  BF::FunctionRef<void () const> f = &Foo;    // OK
  ```

- If `Signature` contains `noexcept`, calling the function has to be a non-throwing operation. This means, the parameter passing (move ctors. and dtors.) don't throw, and the function is itself `noexcept`. Example:
  ```c++
  void Foo();                                 // not noexcept

  BF::FunctionRef<void () noexcept> f = &Foo; // static_assert: 'Foo' is not 'noexcept'
  ```

Unlike in case of standard function wrappers, the initializer of `BF::FunctionRef` can be the name of an overloaded function too. In this case one of the candidates has to satisfy the above conditions, i.e. it has to return `Ret` and has to have parameters `Pars...`, and it has to satisfy the `noexcept` check. Example:

```c++
void Foo();
void Foo(void*);

BF::FunctionRef<void ()> f = &Foo;              // 'f' points to 'void Foo()'
```

You can even overload functions on various `Signature`'s of `BF::FunctionRef`, and call this overload set with an overloaded function:

```c++
void Foo();
void Foo(void*);

void Bar();
void Bar(int);

void Overloaded(BF::FunctionRef<void ()>);
void Overloaded(BF::FunctionRef<void (int)>);

int main()
{
    Overloaded(&Foo);                           // calls the 1st candidate with 'void Foo()'
    Overloaded(&Bar);                           // error: ambiguous call to 'Overloaded'
}
```


### Initializing from a functor

`BF::FunctionRef` will accept a functor (i.e., a function object) on its public interface, if one of its `operator()`'s returns `Ret` and has parameters `Pars...`. Only this property is considered during overload resolution. The rest is checked later by `static_assert`'s. Example:

```c++
struct S {
    void operator()();                          // not noexcept
};

void Overloaded(BF::FunctionRef<void () noexcept>);
void Overloaded(BF::FunctionRef<void (int) noexcept>);

int main()
{
    S s;
    Overloaded(s);                              // static_assert: 'operator()' is not 'noexcept'
}
```

The call in `main` successfully selects the 1<sup>st</sup> candidate. The `static_assert` is issued afterwards.

The following properties are checked by `static_assert`'s:

- `BF::FunctionRef` will call the (callable) initializer expression as if you would call it, that is, it keeps its type, *cv* qualifiers and value category. This call must be well-formed, otherwise the initialization will not compile. Example:
  ```c++
  struct S {
      void operator()();                      // not const
  };

  int main()
  {
      S s;
      BF::FunctionRef<void ()> f = s;         // OK
      s();                                    // "as if you would call it" - also OK

      const S z;
      BF::FunctionRef<void ()> g = z;         // static_assert: cannot initialize 'g' with 'z'
      z();                                    // "as if you would call it" - also ill-formed
  }
  ```

- Additionally, if `Signature` contains `const`, the callable is called through a `const` access path, i.e. as if you would add `const` to it. Example:
  ```c++
  struct S {
      void operator()();                      // not const
  };

  int main()
  {
      S s;
      BF::FunctionRef<void () const> f = s;   // static_assert: cannot initialize 'f' with 's'
      std::as_const(s)();                     // "as if you would add const" - also ill-formed
  }
  ```

- If `Signature` contains `noexcept`, calling the callable has to be a non-throwing operation. This means, the parameter passing (move ctors. and dtors.) don't throw, and the called `operator()` is itself `noexcept`. Example:
  ```c++
  struct S {
      void operator()();                      // not noexcept
  };

  S s;
  BF::FunctionRef<void () noexcept> f = s;    // static_assert: 'operator()' is not 'noexcept'
  ```

The called `operator()` will be always one, that returns `Ret` and has parameters `Pars...`. No other `operator()` will be called by `BF::FunctionRef`.


## Documentation of the public interface


### Default constructor

Same as [constructing from `BF::Bad`](#constructor-from-bfbad).


### Constructor from `BF::Bad`

Constructs an invalid `BF::FunctionRef`. Calling its `operator()` will abort the program. The intended usage is, that the user assigns a callable to it before calling `operator()`.

Note, that the invalid state can be propagated by [copy/move semantics](#special-methods) and [copy from friend](#copy-from-friend).

It is an invalid, and not an empty/null `BF::FunctionRef`. There is no optionality (empty or null state) implemented (although this could be done). `BF::FunctionRef` is therefore not constructible from `std::nullptr_t`, nor can it be compared with `nullptr`.


### Constructor from `BF::Uninitialized`

It's a no-op. Constructs an uninitialized `BF::FunctionRef`. Calling its `operator()` is undefined behavior. The intended usage is, that the user assigns a callable to it before calling `operator()`. Use this ctor. in performance critical places only, otherwise prefer [constructing from `BF::Bad`](#constructor-from-bfbad).

Note, that the uninitialized state can be propagated by [copy/move semantics](#special-methods) and [copy from friend](#copy-from-friend).


### Constructing/assigning from functions and functors

This works as described in [Initialization](#initialization).


### Copy from friend

`BF::FunctionRef` is copyable from a `BF::FunctionRef` with the same `Ret (Pars...)`, but different `const`<sub>op</sub> and `noexcept`<sub>op</sub> in the `Signature`. The source `BF::FunctionRef` has to be more constrained than the target. Example:

```c++
void Foo();
BF::FunctionRef<void ()>       source  = &Foo;
BF::FunctionRef<void () const> sourceC = &Foo;

BF::FunctionRef<void ()>       target  = sourceC;   // OK, refers to 'Foo'
BF::FunctionRef<void () const> targetC = source;    // error: source is less constrained
```

The target `BF::FunctionRef` will not refer to the source `BF::FunctionRef`, rather it will directly refer to the callable referred by the source.


### `operator()` method

Calls the pointed callable.


### `ConstCast()` method, with no template arguments<a id="constcast-method"/>

Removes the `const` (but not the `volatile`) qualifier from a `const`-qualified `BF::FunctionRef`. It doesn't change the `Signature`. The method triggers a `static_assert`, if the cast is unnecessary. Example:

```c++
const BF::FunctionRef<void ()> f;
f.ConstCast();              // OK

BF::FunctionRef<void ()> g;
g.ConstCast();              // static_assert, because 'g' is already not const

const BF::FunctionRef<void () const> h;
h.ConstCast();              // static_assert, because 'h()' can be called without any casting
```

The value category is preserved. Example:

```c++
const BF::FunctionRef<void ()> f;
f.ConstCast();              // returns 'BF::FunctionRef<void ()>&' referring to 'f' (lvalue)
std::move(f).ConstCast();   // returns 'BF::FunctionRef<void ()>&&' referring to 'f' (rvalue)
```


### `ConstCast<ToSignature>()` method

Adds or removes `const` (but not `volatile`) to/from the `Signature` of `BF::FunctionRef`. `ToSignature` is the new signature.

- If `Signature` is/is not `const`, `ToSignature` must also be/not be `const`.
- If `Signature` is/is not `noexcept`, `ToSignature` must also be/not be `noexcept`.
- The number of function parameters in `ToSignature` must be the same as in `Signature`.
- The return type and the parameter types of `ToSignature` must differ only in the presence or absence of `const` from the corresponding type in `Signature`. The difference can occur at any level behind `*`, `&` or `&&`, but unlimited levels of C arrays (`[]`, `[N]`) are currently not supported.

Examples:

```c++
BF::FunctionRef<void ()> f1;
f1.ConstCast<void () const>();                                          // error

BF::FunctionRef<signed ()> f2;
f2.ConstCast<unsigned ()>();                                            // error

BF::FunctionRef<volatile int&& (int*&, int const* const&)> f3;
f3.ConstCast<const volatile int&& (int const* const&, int*&)>();        // OK
```

A typical usage of `ConstCast<ToSignature>()` is when you implement a pair of methods (non-`const` and `const`), and call one from the other. Example:

```c++
template <class Type>
struct MyContainer {
    void Enumerate(BF::FunctionRef<void (Type&)> f) {
        Enumerate(f.ConstCast<void (const Type&)>());
    }

    void Enumerate(BF::FunctionRef<void (const Type&)> f) const {
        for (const Type& item : *this)
            f(item);
    }
};
```

The *cv* qualifiers and the value category of `BF::FunctionRef` is preserved. Example:

```c++
const BF::FunctionRef<const void* ()> f;
f.ConstCast<void* ()>();              // 'const BF::FunctionRef<void* ()>&' referring to 'f'
std::move(f).ConstCast<void* ()>();   // 'const BF::FunctionRef<void* ()>&&' referring to 'f'
```


### Special methods

The copy/move constructors, copy/move assignment, and the destructor are trivial.


## Deduction guide

`BF::FunctionRef` can deduce its `Signature` template parameter from the initializer under the following conditions:
- If the initializer is a function, it must not be overloaded.
- If the initializer is a class type, it must have exactly one `operator()`.

The deduced `Signature` will be the following:
- If the initializer is a function, the type of the function, including the `noexcept` specifier. `Signature` therefore cannot be `const`.
- If the initializer is a class type, the type of the `operator()` removing the class itself (thus turning a member function pointer into a function type), the `volatile` qualifier, and the ref-qualifiers, but keeping the `const` qualifier and the `noexcept` specifier.

Example:

```c++
void Foo() noexcept;

struct S {
    void operator()() const volatile& noexcept;
};

S s;

BF::FunctionRef f1 = &Foo;          // 'Signature' == 'void () noexcept'
BF::FunctionRef f2 = s;             // 'Signature' == 'void () const noexcept'
```


## Remarks on the implementation

The class template is named `FunctionRef` and not `FunctionPtr` for two reasons:
- it is not nullable,
- it can be initialized/assigned from functors (and not pointers to them).

The implementation is [freestanding](https://timsong-cpp.github.io/cppwp/n4950/intro.compliance.general#7), i.e. it doesn't use operating system features, like memory allocation.

To compile it, C++23 features must be enabled.

Tested on Visual Studio 2022 v17.14.20 with `/std:c++latest`.


## Standard function wrappers and views<a id="std-wrappers"/>

| Type             | Name                      | Link to cppreference.com | Essay/proposal |
| ---------------- | ------------------------- | ------------------------ | -------------- |
| Function wrapper | `std::function`           | [since C++11](https://en.cppreference.com/w/cpp/utility/functional/function)           | [std::function and Beyond](https://wg21.link/N4159) |
| Function wrapper | `std::move_only_function` | [since C++23](https://en.cppreference.com/w/cpp/utility/functional/move_only_function) | [P0288](https://wg21.link/P0288) |
| Function wrapper | `std::copyable_function`  | [since C++26](https://en.cppreference.com/w/cpp/utility/functional/copyable_function)  | [P2548](https://wg21.link/P2548) |
| Function view    | `std::function_ref`       | [since C++26](https://en.cppreference.com/w/cpp/utility/functional/function_ref)       | [P0792](https://wg21.link/P0792) |
