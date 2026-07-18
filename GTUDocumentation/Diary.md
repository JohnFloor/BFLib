# `Diary.hpp`

This file contains facilities to log events such as object creation, copying, moving and destruction. The event log can then be tested against an expected result. This is useful when testing containers and value wrappers.


## `GTU::Push()`

The function `GTU::Push()` can be used to push characters to the event log.

The macro `GTU_XD(expectedEvents)` creates a scope at the end of which we expect the value of the log to be `expectedEvents`, which is a string. `GTU_XD` stands for Google Test Utilities, Expect Diary.

Example:

```c++
#include "gtest/gtest.h"
#include "GTU/Diary.hpp"

TEST(Example, One)
{
    GTU_XD("ab") {              // we expect that the event log will contain "ab" at the end of the scope
        GTU::Push('a');         // push 'a' into the event log
        GTU::Push('b');         // push 'b' into the event log
    }
}
```

Nesting `GTU_XD()` scopes is not supported.

Using the event log in parallel threads will work. Every thread has a separate event log.


## `GTU::Diary`

This is a simple class which pushes the following events (characters) into the event log:

| Function              | The pushed event |
| ----------------------| ---------------- |
| Default ctor.         | `+`              |
| Copy ctor./assignment | `C`              |
| Move ctor./assignment | `M`              |
| Dtor.                 | `-`              |

The assignments log these events on self-copy and self-move too.

The class is an empty class.


## Usage example

In the following example we write a simple `Wrapper` class and test it.

```c++
#include <new>
#include <utility>
#include "gtest/gtest.h"
#include "GTU/Diary.hpp"


template <class Type>
class Wrapper {
public:
    Wrapper() {
        ::new (mBuffer) Type();
    }

    Wrapper(Wrapper&& source) noexcept {
        ::new (mBuffer) Type(source.Get());             // we forgot 'std::move()'
    }

    ~Wrapper() {
        Get().~Type();
    }

private:
    Type& Get() {
        return reinterpret_cast<Type&>(mBuffer);
    }

    alignas(Type) char mBuffer[sizeof(Type)];
};


TEST(Wrapper, DefaultCtor)
{
    GTU_XD("+-") {
        Wrapper<GTU::Diary> w;
    }
}


TEST(Wrapper, MoveCtor)
{
    Wrapper<GTU::Diary> s;
    GTU_XD("M-") {                                      // the test fails: expected event log is "M-", actual is "C-"
        Wrapper<GTU::Diary> t = std::move(s);
    }
}
```


## Design decisions

* When copying and moving `GTU::Diary` the ctor. and the assignment is deliberately not distinguished. In practice there are cases where a container can freely choose between one and the other. We consider this an implementation detail of the container and do not want the tests to fail if the container is rewritten, e.g., from calling the copy ctor. to calling the copy assignment. The other reason for not distinguishing them is simplicity.
* The macro name `GTU_XD()` is deliberately short to make it suitable for writing one-line tests.
