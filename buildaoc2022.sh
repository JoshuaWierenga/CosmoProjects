#! /bin/sh

rm -rf o/third_party/aoc/ 2>/dev/null
make -j16 o//third_party/aoc/aoc.com

if [ "$1" = --run ]; then
    echo
    o/third_party/aoc/aoc.com
fi