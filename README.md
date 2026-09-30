# R-TYPE

## Compilation

```bash
cmake -S . -B build
cmake --build build
```

## linter

```bash
clang-tidy -p build --fix-errors  src/main.cpp
```
