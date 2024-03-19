# Argparse Tests

## Linux

```bash
$ mkdir build
$ cd build
$ cmake ../.
$ make
$ ./argparse_tests
```

## Windows

1. Generate Visual Studio solution

```bash
$ mkdir build
$ cd build
$ cmake ../. -G "Visual Studio 15 2017"
```

2. Open ARGPARSE.sln
3. Build argparse_tests in RELEASE | x64
4. Run argparse_tests.exe
