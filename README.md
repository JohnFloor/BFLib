### Directory `BF`

A **B**asic **F**acilities library. It contains the following:
- [`FunctionRef.hpp`](BFDocumentation/FunctionRef.md): A type-erased function view.
- [`Hash.hpp` and `HashRange.hpp`](BFDocumentation/Hash.md): Easier hashing for Standard Library unordered containers.
- Other undocumented minor features.


### Directory `BFDocumentation`

Documentation for some parts of the `BF` library. They are all referenced in this file.


### Directory `BFTest`

Unit tests for `BF`. It also tests incorrect usages of the library, i.e. it contains expected diagnostic messages for some cases of improper usage. These are in comments, marked with [`[CompilationError]`](BFDocumentation/CompilationErrorTags.md) tags.


### Directory `CheckCE`

It contains a Python script [`CheckCE.py`](BFDocumentation/CheckCE.md) that automates checking the mentioned `[CompilationError]` tags.
