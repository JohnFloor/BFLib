# Testing compilation errors

A distinctive feature of the `BFTest` directory is the testing of compilation errors.

One of the focal points of the `BF` library is to provide clear and understandable error messages. These are implementation decisions, just like the normal operation of the library (when usage compiles), and therefore they also need to be tested.

In order to test compilation errors, we need to trigger them. This, however, cannot be part of the test code as the tests wouldn't compile. Therefore, the line that is expected not to compile is commented out, and the expected diagnostic message is after it, marked with a tag.


## The `[CompilationError]` tag

Example:

```c++
BF::FunctionRef<void ()> f;

// f.ConstCast();       // [CompilationError]: No need to cast away constness, '*this' is already not const.
```

 This test means the following:

- If you uncomment this line (remove the first `//`), and
- keep the rest of the .cpp file unchanged, and
- compile it with the same settings as in the project,

then

- the file is expected not to compile, and
- the first diagnostic message is expected to contain the string after the tag (verbatim).


## The <code>[CompilationError-<i>SolutionConf</i>]</code> tag

The tag can optionally contain the name of a solution configuration after a `-`. Example:

```c++
// BF_ASSERT("C"++);    // [CompilationError-Debug]: '++' needs l-value
```

In addition to the simple tag (without a solution configuration), this test means the following:

- The test has an expectation only if compiled with the given configuration.

In the example above, since the assert is not compiled in `Release` configuration, a `[CompilationError]` tag would report in `Release` build that the line unexpectedly compiles. Therefore, we need to use the `[CompilationError-Debug]` tag.


## Automating tag checking

The [`CheckCE.py`](CheckCE.md) script automates checking these tags.
