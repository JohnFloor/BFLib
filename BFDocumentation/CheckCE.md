# CheckCE.py

Checks `[CompilationError]` tags in .cpp files.


## Usage

```sh
CheckCE.py
CheckCE.py path
CheckCE.py path solution_conf
CheckCE.py path solution_conf n_worker_threads
```

| Argument           | Meaning                                                      | Default value               |
| ------------------ | ------------------------------------------------------------ | --------------------------- |
| `path`             | A directory or a single .cpp file to process.                | `.` (the current directory) |
| `solution_conf`    | A solution configuration. Must be present in the .sln file.  | `Debug`                     |
| `n_worker_threads` | Number of worker threads to use for the check, between 1-20. | `10`                        |

The .sln file will be searched in the following directories: `path`, `path\..`, `path\..\..`, etc.
The search stops at the first match. It is an error if that directory contains more than one .sln file.

A C++ project directory is a directory in which there is a .vcxproj file. It is an error, if it contains
more than one .vcxproj file. It must be the same directory or a child of the solution directory.

`path` must be in a project directory (descendant-or-self).


## Print help

```sh
CheckCE.py /?
```


## Exit codes

- `0`: The check passed for all (0 or more) .cpp files.
- `1`: The check failed at least once.
- `2`: Technical error (e.g. directory/file does not exist).
