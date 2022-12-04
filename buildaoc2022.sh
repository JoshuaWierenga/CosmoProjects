#! /bin/sh

rm o/third_party/aoc/aoc.com 2>/dev/null
make -j16 o//third_party/aoc/aoc.com

if [ "$1" = --run ]; then
    echo
    o/third_party/aoc/aoc.com
fi