# Easier hashing

This facility (implemented in `BF/Hash.hpp` and `BF/HashRange.hpp`) enables a syntactically lighter way of making your class hashable, much lighter than specializing `std::hash` and struggling with combining the hash values on your own.


## Making a class hashable

You can make the class `Person` hashable by implementing exactly one of the following:
  - A public `Person::BF_GetHash() const` method.
  - A `BF_GetHash(const Person&)` function in the same header and namespace as `Person`.

Both of these have to return with (cv/ref-unqualified) `BF::Hash`. The implementation of the method or function is very simple: just construct the return value from the member variables you want to hash (which is typically all of them). `BF::Hash`'s ctor. will hash all its arguments, and combine their hash values.

`Person.hpp` should look like this, in case of choosing to make the type hashable with a `BF_GetHash()` method:

```c++
#pragma once
#include "BF/Hash.hpp"

namespace N {
    class Person {
    public:
        BF::Hash BF_GetHash() const {
            return { m1, m2, m3 };
        }

    private:
        Type1 m1;
        Type2 m2;
        Type3 m3;
    };
}
```

And, like this, in case of a `BF_GetHash()` function:

```c++
#pragma once
#include "BF/Hash.hpp"

namespace N {
    class Person {
    public:
        const Type1& GetM1() const { return m1; }
        const Type2& GetM2() const { return m2; }
        const Type3& GetM3() const { return m3; }

    private:
        Type1 m1;
        Type2 m2;
        Type3 m3;
    };

    inline BF::Hash BF_GetHash(const Person& value)
    {
        return { value.GetM1(), value.GetM2(), value.GetM3() };
    }
}
```

Alternatively, the `BF_GetHash()` function can access the members directly, if `Person` declares it as a friend.

After this, assuming that `Person` has an `operator==` too, you can put the class into a Standard Library unordered container:

```c++
#include <unordered_set>
#include <unordered_map>
#include "Person.hpp"

int main()
{
    std::unordered_set<N::Person>      set;
    std::unordered_map<N::Person, int> map;
}
```

This works, because `BF/Hash.hpp` contains the necessary `std::hash` specialization. This specialization detects the presence of a public `BF_GetHash()` method or a `BF_GetHash()` function, and calls it.

By the same token, providing both the method and the function would be ambiguous. The library reports this with a `static_assert`.

On the other hand, don't specialize `std::hash` for a class for which you use `BF_GetHash()` method or function. Your `std::hash` specialization would be a better match than the one in `BF/Hash.hpp`, and the `BF_GetHash()` method and function would be ignored. Even the mentioned ambiguity wouldn't be reported.


## Effect on derived classes

The `BF_GetHash()` function doesn't affect derived classes:

```c++
struct Base {};

inline BF::Hash BF_GetHash(const Base&) { /*...*/ }         // makes 'Base' hashable

struct Derived : Base {};                                   // not hashable
```

However, the `BF_GetHash()` method will be inherited, therefore the derived class will also be hashable (if the inheritance is `public`). This is sometimes desirable, sometimes not. To make the derived class not hashable `= delete` the `BF_GetHash()` method in it:

```c++
struct Base {
    BF::Hash BF_GetHash() const { /*...*/ }                 // makes 'Base' hashable
};

struct Derived1 : Base {};                                  // also hashable

struct Derived2 : Base {                                    // not hashable
    BF::Hash BF_GetHash() const = delete;
};
```

Note, that you have to `= delete` the method, and not the function.


## 3<sup>rd</sup> party libraries

This chapter proposes a method for how to make 3<sup>rd</sup> party library classes and class templates hashable. You cannot modify a 3<sup>rd</sup> party library header.

Suppose, the 3<sup>rd</sup> party library is called `Lib`, and its headers are under the `Lib/` folder. Apply the following process.
* Adapt the `Lib` headers in which there are classes or class templates you want to make hashable. This means, e.g. for `Lib/Header.hpp` the following:
  * You create an `Adapted/Lib/Header.hpp`, the adaptation,
  * in which you #include `BF/Hash.hpp` and `Lib/Header.hpp`, and
  * make the appropriate `Lib` types hashable (see the following subchapters).
* After this you don't ever #include `Lib/Header.hpp`, instead, you always #include `Adapted/Lib/Header.hpp`.

Since the 3<sup>rd</sup> party library header is not modifiable, you won't be able to use the `BF_GetHash()` method.
* For classes use the `BF_GetHash()` function,
* for class templates specialize `std::hash`.


### Classes

Suppose, in `Lib/Id.hpp` you want to make `Lib::Id` hashable. `Lib/Id.hpp` looks like this:

```c++
#pragma once

namespace Lib {
    struct Id {
        unsigned id = 0;
    };
}
```

Your adaptation, `Adapted/Lib/Id.hpp`, will then look like this:

```c++
#pragma once
#include "BF/Hash.hpp"
#include "Lib/Id.hpp"

namespace Lib {
    inline bool operator==(const Id& leftOp, const Id& rightOp)
    {
        return leftOp.id == rightOp.id;
    }

    inline BF::Hash BF_GetHash(const Id& value)
    {
        return { value.id };
    }
}
```


### Class templates

Suppose, in `Lib/Counted.hpp` you want to make `Lib::Counted<Type>` hashable for all `Type`'s which are themselves hashable. `Lib/Counted.hpp` looks like this:

```c++
#pragma once

namespace Lib {
    template <class Type>
    struct Counted {
        Type     type{};
        unsigned count{};
    };
}
```

Your adaptation, `Adapted/Lib/Counted.hpp`, will then look like this:

```c++
#pragma once
#include <type_traits>
#include "BF/Hash.hpp"
#include "Lib/Counted.hpp"

namespace Lib {
    template <std::equality_comparable Type>
    bool operator==(const Counted<Type>& leftOp, const Counted<Type>& rightOp)
    {
        return leftOp.type == rightOp.type && leftOp.count == rightOp.count;
    }
}

template <BF::StdHashable Type>
struct std::hash<Lib::Counted<Type>> {
    [[nodiscard]]
    static std::size_t operator()(const Lib::Counted<Type>& value) {
        return BF::Hash(value.type, value.count);
    }
};
```

`BF::Hash` will convert to `std::size_t` inside `std::hash` specializations (but, outside of it it won't).


## Hashing `const char*` strings, ranges, and raw memory

These are not hashable by default. You cannot just pass the member variable to the `BF::Hash` ctor. Instead, wrap it in a helper class, and pass `Helper(mVar)` to the `BF::Hash` ctor. as follows.

**`const char*` strings**. Hashing the `const char*` member variable will hash the pointer itself. To hash the pointed C string, construct an `std::string_view` from it.

**Ranges**. Hashing random access ranges is supported by `BF/HashRage.hpp`. To hash a random access range member variable, construct a `BF::HashRange` object from it.

**Raw memory**. You can hash the memory representation of a type with `BF::HashRawMemory`, which is also in `BF/HashRage.hpp`.
* `BF::HashRawMemory(value)` will cast `value` to its byte representation, and hash that as a range of `std::byte`'s. Can be used only for types with unique object representations.
* `BF::HashRawMemory(begin, size)` will hash the memory range beginning at pointer `begin` and of size `size` (which is in bytes).
* `BF::HashRawMemory(begin, end)` will hash the memory range [`begin`, `end`). The `begin` and `end` pointers have to be of the same type, and that type has to have unique object representations.

Example usage:

```c++
#pragma once
#include <string_view>
#include <vector>
#include "BF/HashRage.hpp"

struct ComplexClass {
    class UniqueObjReps {
        Int64 a;
        Int32 b[2];
    };

    BF::Hash BF_GetHash() const {
        return { mInt,
                 std::string_view(mStr),
                 BF::HashRange(mRange),
                 BF::HashRawMemory(mUniqueObjReps),
                 BF::HashRawMemory(mMem1Begin, mMem1Size),
                 BF::HashRawMemory(mMem2Begin, mMem2End) };
    }

    UInt64              mInt;
    const char*         mStr;
    std::vector<UInt16> mRange;
    UniqueObjReps       mUniqueObjReps;
    std::byte*          mMem1Begin;
    std::size_t         mMem1Size;
    std::byte*          mMem2Begin;
    std::byte*          mMem2End;
};
```


## Best practices
* In general, using the `BF_GetHash()` method is recommended over the function, as its implementation will be shorter, and it can be used in class templates too.
* If you introduce `BF_GetHash()` into your project, replace all `std::hash` specializations, and create adapted headers for 3<sup>rd</sup> party library headers.
* Don't mix `std::hash` specializations with `BF_GetHash()`. This would eventually lead to subtle bugs. As mentioned earlier, the specialization _silently_ overrides the `BF_GetHash()` method and function, therefore sooner or later somebody will modify `BF_GetHash()`, but it will have no effect.
* Always make the type hashable in its header. This rule applies even if you use `std::hash` only. Violating it can result in ODR violation, which leads to nasty bugs. E.g., you insert an element into an `std::unordered_set`, and a few lines later `contains()` cannot find it ([live demo](https://godbolt.org/z/719GTcrfe)). The same is true for operators: declare operators in the class's header, and never locally in the .cpp file where the class is used.
* The `BF_GetHash()` method should not be `virtual`. In fact, a hashable type shouldn't be polymorphic at all.
* The definition of the `BF_GetHash()` method or function should be in the class's header file, `inline`, so that the compiler can inline it, and generate more efficient code.
* You shouldn't call `std::hash<Type>()(value)` yourself. If you need a fingerprint of your type, that should be implemented separately, with separate implementation guarantees. The implementation of hashing or hash combination can change any time. E.g., writing it to a file, or sending it through a socket can lead to unexpected results. The other program is not guaranteed to use the same hashing or hash combination algorithm.
* Good to know: If you change the order of `BF::Hash` ctor. arguments, the resulting hash changes.
