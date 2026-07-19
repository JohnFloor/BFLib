# `BF::Ref`

`BF::Ref` is a smart reference that brings const-correctness to reference classes.


## Reference classes and the const-correctness problem

By _reference class_ we mean a class that itself represents a reference rather than a value.

Good examples are classes that contain an OS handle, with methods that call the corresponding OS API functions using that handle (and other function arguments). Example:

```c++
class Window {
public:
    Window(HWND hwnd)                               : mHwnd(hwnd) {}

    std::string GetTitle() const                    { GetWindowTextW(mHwnd, ...); }
    void        SetTitle(std::string_view newTitle) { SetWindowTextW(mHwnd, ...); }

private:
    HWND        mHwnd;
};
```

This class has reference semantics. Copying `Window` objects copies the reference:

```c++
Window w1 = hwnd;
Window w2 = w1;
// now 'w1' and 'w2' refer to the same window
```

Using this `Window` class is convenient, except that const-correctness won't work. We do have both non-`const` and `const` methods, but a `const Window` can be converted to a `Window`, which loses the `const` qualifier:

```c++
const Window w1 = hwnd;
// we can use only const methods on 'w1'

Window w2 = w1;
// unfortunately, this compiles and allows us to call non-const methods
```

Note that we don't have a class representing the referred value. The window value is only a conceptual entity; the OS does not expose it directly. We can only interact with it through HWNDs and C API functions.


## Solution with `BF::Ref`

To address the above const-correctness problem, we put two `using` declarations after the definition of class `Window`:

```c++
using WindowRef      = BF::Ref<Window>;
using WindowConstRef = BF::Ref<const Window>;
```

And, we don't use the `Window` class directly. Instead, we only use `WindowRef` and `WindowConstRef`. Examples:

```c++
void PrintWindowTitles(const std::vector<WindowConstRef>& windows)
{
    for (const WindowConstRef w : windows)
        std::println("{}", w->GetTitle());
}


void ModifyWindowTitles(const std::vector<WindowRef>& windows)
{
    for (const WindowRef w : windows)
        w->SetTitle('[' + w->GetTitle() + ']');
}
```

Converting a `WindowConstRef` to a `WindowRef` will give a compilation error:

```c++
WindowConstRef w1 = hwnd;
// we can use only const methods on 'w1'

WindowRef w2 = w1;
// static_assert: Cannot construct/assign from 'BF::Ref<SourceType>'. Conversion would lose a const qualifier.
```


## Design decisions

* An alternative would be to require the client to write two classes (`Window` and `ConstWindow`), but keeping all methods in a single class and solving the const-correctness problem centrally in the library seemed to be a cleaner solution.
* The class is designed not to impose any performance penalty compared to using the underlying handle in C style.
* Since `BF::Ref` does not know the class that represents the referred value (even if it exists), it has no `operator*`. Note that the template parameter of `BF::Ref` is the reference class, not the referred-to class.
