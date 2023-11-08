![Cosmopolitan Honeybadger](/../../../../jart/cosmopolitan/blob/master/usr/share/img/honeybadger.png)

# Miscellaneous Cosmopolitan Projects
[Link to Cosmopolitan](https://github.com/jart/cosmopolitan)
<br /><br />

## Float precision testing
I made this because I wanted to know the smallest increase allowed for a given power of two with different float types.
I could only find incomplete tables online but full ones for binary32 and binary64 probably do exist.
Currently the program only supports those two but should later work for x86 80 bit as well as decimal32 and decimal64 at a minimum.
It's a single file that should be builable with basically anything, I used cosmocc but did not use any cosmopolitan specific features afaik.
<br /><br />

# Dynamic shared library loading
Made mostly to help me understand shared libraries and how to to load them at runtime via dlopen/dlsym and LoadLibrary/GetProcAddress.
The provided testprog.c should be buildable with glibc for linux, cosmo for linux and windows with seperate libraries and cosmo for
linux and windows with the libaries embedded into the binary.
<br /><br />

# OLD, require build.sh

## AoC 2022
[Link to AoC](https://adventofcode.com/2022)


## X-TOY
Writing an assembler and potentially a c compiler for the X-TOY cpu.\
[Visual X-TOY Simulator](https://lift.cs.princeton.edu/xtoy/)\
[X-TOY Documentation](https://www.comscigate.com/cs/IntroSedgewick/50machine/toy.pdf)
