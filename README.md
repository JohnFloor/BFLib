### Directory `BF`

A **B**asic **F**acilities library. It contains the following:
- [`FunctionRef.hpp`](BFDocumentation/FunctionRef.md): A type-erased function view.
- [`Hash.hpp` and `HashRange.hpp`](BFDocumentation/Hash.md): Easier hashing for Standard Library unordered containers.
- [`Ref.hpp`](BFDocumentation/Ref.md): A smart reference that brings const-correctness to reference classes.
- Other undocumented minor features.


### Directory `BFDocumentation`

Documentation for some parts of the `BF` library. They are all referenced in this file.


### Directory `BFTest`

Unit tests for `BF`. It also tests incorrect usages of the library, i.e. it contains expected diagnostic messages for some cases of improper usage. These are in comments, marked with [`[CompilationError]`](BFDocumentation/CompilationErrorTags.md) tags.


### Directory `CheckCE`

It contains a Python script [`CheckCE.py`](BFDocumentation/CheckCE.md) that automates checking the mentioned `[CompilationError]` tags.


### Directory `GTU`

**G**oogle **T**est **U**tilities library. It contains the following:
- [`Diary.hpp`](GTUDocumentation/Diary.md): Contains facilities to log events such as object creation, copying, moving and destruction. The event log can then be tested against an expected result. This is useful when testing containers and value wrappers.
- Other undocumented minor features.


### Directory `GTUDocumentation`

Documentation for some parts of the `GTU` library. They are all referenced in this file.


### Directory `GTUTest`

Unit tests for `GTU`.
