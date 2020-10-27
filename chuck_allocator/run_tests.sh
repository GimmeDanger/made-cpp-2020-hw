#!/bin/bash

set -e

g++ -std=c++17 -I./ tests/tests.cpp -o runner

valgrind --tool=memcheck --gen-suppressions=all --leak-check=full --leak-resolution=med --track-origins=yes --vgdb=no ./runner 500

rm runner

echo All tests passed!
