#!/bin/sh
efiFullSize=1440

# this script produces floppy images that work in qemu and vmware workstation, perhaps other vmms as well

# set to false to produce a smaller uefi image that in qemu can only started via the efi shell, other vmms do not appear to care
qemuUefiDirectBoot=true

# TODO check if these can be removed
buildLinux=false
buildBios=false
buildUefi=false

# can be run in three ways
# 1. 3 arguments:
# $1: path to the cosmo git repo
# $2: path to a cosmo binary relative to the cosmo git repo
# $3: target to build for: linux, bios or uefi
# 2. no arguments, builds hardcoded program for bios and uefi
# 2. 1 argument = clean, cleans up and quits

# TODO restore support for writing images to usbs
# TODO find a way to output one image that works on bios and uefi
if [ "$#" -eq 3 ] ; then
    sourceFileName=${2##*/} # dir/.../dir/file.ext -> file.ext
    baseSourceFileName=${sourceFileName%.c} # file.c -> file

    makePath=o//${2%.c}.com # path to output file relative to the cosmo git repo

    if [ "$3" = linux ] ; then
        buildLinux=true
        destinationLinux=$baseSourceFileName.com
    elif [ "$3" = bios ] ; then
        buildBios=true
        destinationBios=$baseSourceFileName.bios.flp
    elif [ "$3" = uefi ] ; then
        buildUefi=true
        destinationUefi=$baseSourceFileName.uefi.flp
    fi
elif [ "$#" -eq 0 ] ; then
    set -- ../cosmo/cosmopolitan
    makePath=o//examples/vga2.com # path to output file relative to the cosmo git repo

    buildBios=true
    buildUefi=true

    destinationLinux=cosmo.com
    destinationBios=cosmo.bios.flp
    destinationUefi=cosmo.uefi.flp
elif [ "$1" = clean ] ; then
    rm -rf ./*.com ./*.flp
    exit 0
else
    exit 1
fi

binaryPath=$1/$makePath # path to output file relative to the current directory
(cd "$1" && make "$makePath" -j"$(($(nproc) + 1))")

if [ "$buildLinux" = true ] ; then
    cp "$binaryPath" "$destinationLinux"
fi

if [ "$buildBios" = true ] ; then
    cp "$binaryPath" "$destinationBios"
fi

# Based on https://gitlab.com/-/snippets/2548098
if [ "$buildUefi" = true ] ; then
    if [ "$qemuUefiDirectBoot" = true ] ; then
        dd status=none if=/dev/zero of="$destinationUefi" bs=1024 count="$efiFullSize"
    else
        dd status=none if=/dev/zero of="$destinationUefi" bs=1024 count=1
    fi
    mkdosfs -F 12 -g 1/8 -M 0xfe -r 16 -s 2 --mbr=y "$destinationUefi" "$efiFullSize"
    # fix the partition type byte (this may not be needed, but is good to do)
    printf '\xef' | dd status=none of="$destinationUefi" bs=1 seek=450 conv=notrunc
    mmd -i "$destinationUefi" ::efi
    mmd -i "$destinationUefi" ::efi/boot
    mcopy -i "$destinationUefi" "$binaryPath" ::efi/boot/bootx64.efi
    sync
fi
