# Copy all fuzzer executables to $OUT/
$CXX $CFLAGS $LIB_FUZZING_ENGINE \
  $SRC/argparse/.clusterfuzzlite/fuzz_argparse.cpp \
  -o $OUT/fuzz_argparse \
  -std=gnu++17 \
  -I$SRC/argparse/include