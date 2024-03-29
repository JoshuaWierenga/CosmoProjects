#! /bin/sh
# Ubuntu 22.04 cosmo third party library setup 0.1.0

version="0.1.0"

usage()
{
    echo "Ubuntu 22.04 cosmo third party library setup $version
Also runs on wsl where it automatically sets up binfmt to ensure the build doesn't fail
Usage: $0 [general flags] [project to run]

General Flags:
-?, -h: Display this usage information and then quit.
-v: Display version history and then quit.
-c: Remove overlay folders and then quit.
-r: Recreate overlay folders.
-s: Skip wsl binfmt configuration.
-u: Update makefile includes.

The project to run must match the name of a project folder, the name of its mk file and an executable generated by it."
    exit
}

version()
{
    echo "Ubuntu 22.04 cosmo third party library setup $version
Changelog:"
    exit
}

# Helpers

isSubmodule()
{
    output=$(cd cosmopolitan/git && git rev-parse --show-superproject-working-tree)

    if [ -n "$output" ] ; then
        true
    else
        false
    fi
}


setupWsl()
{
    if grep -qi microsoft /proc/version && [ ! -f /proc/sys/fs/binfmt_misc/APE ] && [ -z "$skipWSL" ]; then
      sudo sh -c "echo ':APE:M::MZqFpD::/bin/sh:' >/proc/sys/fs/binfmt_misc/register"
    fi
}

# Overlay management

removeOverlay()
{
    mountpoint -q cosmopolitan/build && sudo umount cosmopolitan/build
    rm -rf cosmopolitan/build cosmopolitan/overlay
}

createOverlay()
{
    mountpoint -q cosmopolitan/build && return

    if [ ! -d cosmopolitan/git ] || ! isSubmodule; then
        echo "Cosmopolitan submodule has not been initialised."
        exit
    fi

    mkdir -p cosmopolitan/build cosmopolitan/overlay/temp cosmopolitan/overlay/upper

    # TODO Figure out how to use with unshare on wsl
    # sudo can be replaced on most linux systems with unshare -rm, however, for some reason this fails on wsl
    sudo mount -t overlay -o lowerdir=cosmopolitan/git,upperdir=cosmopolitan/overlay/upper,workdir=cosmopolitan/overlay/temp overlay cosmopolitan/build

    # TODO Fix build flake on overlay
    # This test fails on overlayfs so get rid of it for now
    rm cosmopolitan/build/test/libc/stdio/tmpfile_test.c

    updateIncludes=true
}

updateOverlay()
{
    for f in *; do
        [ ! -d "$f" ] || [ "$f" = "cosmopolitan" ] && continue
        cp -r "$f" cosmopolitan/build/third_party/

        if [ "$f" = "$1" ] ; then
            projectExists=true
            rm -rf "cosmopolitan/build/o//third_party/$f" 2>/dev/null
        fi
    done

    [ -z "$updateIncludes" ] && return

    cp cosmopolitan/git/Makefile cosmopolitan/build/Makefile

    # TODO Allow specifying where in the makefile to put the include so that specific includes can be added earlier on
    # TODO Allow specifying the name of the mk file
    for f in *; do
        [ ! -d "$f" ] || [ "$f" = "cosmopolitan" ] && continue
        sed -i "/include test\/test.mk/a include third_party\/$f\/$f.mk" cosmopolitan/build/Makefile
    done
}

# based on https://www.shellscript.sh/tips/getopts/

unset skipWSL updateIncludes projectExists

while getopts "hvrcsu" c ; do
    case $c in
        h|\?) usage ;;
        v) version ;;
        r) removeOverlay ;;
        c) removeOverlay; exit ;;
        s) skipWSL=true ;;
        u) updateIncludes=true ;;
    esac
done

shift $((OPTIND-1))

setupWsl
createOverlay
updateOverlay "$1"

cd cosmopolitan/build || exit

if [ -z $projectExists ] ; then
    make -j16
else
    #TODO Allow specifying executable name
    make -j16 "o//third_party/$1/$1.com"
    "o//third_party/$1/$1.com"
fi
cd ../..