#!/bin/bash

set -e

g++ -std=c++17 -I./ tests/tests.cpp -o runner
./runner

rm runner

echo All tests passed!
