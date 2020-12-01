#!/bin/bash

set -e

g++ -std=c++17 -I./ src/main.cpp -o function_test
./function_test

rm function_test

echo All tests passed!
