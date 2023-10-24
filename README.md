# linker-is-bloat

A simple and useless mach-o (64-bit) parser to load functions.

## Usage

Compile the C source to load functions from using the following:

```
$ clang -c lib.c -o lib.o
```

The `-c` flag stops the compiler after the compilation stage which prevents invoking the linker, we will manually load the functions in the compiled file.

Compile the main program using the following:

```
$ clang main.c -o main
```

With both compiled and in same directory, you can simply run the main executable.

## Resources

-   [OSX ABI Mach-O File Format Reference](https://github.com/aidansteele/osx-abi-macho-file-format-reference)
