#! /bin/sh
## user variables
# paths
cosmoFolder=cosmopolitan/git
subFolder=examples

# behaviour
testLinux=true # used as control
testBios=true
testUefi=true
createResultsTable=true
guiQemu=true # if false the script adds -nographic to qemu calls to run it entirely within the terminal

# skip known bad programs that break the testing script
# requires spaces between all elements as well as between the quotes and the first and last items
skipListLinux=" crashreport2 "
skipListBios=""
skipListUefi=""

# arguments
# $1. optional source path relative to cosmoFolder, if set then it just runs the given program without writing to the table

# internal variables
preppedSource=false

# prepares a binary for running on bios and uefi
# $1: path to a cosmo c source file (including name)
# $2: name of the same cosmo c source file
prepsource() {
    if [ "$preppedSource" = true ] ; then
        return
    fi

    cp "$1" "$2"

    # ensure the program displays console output and doesn't try to quit on completion
    # emits are done reverse order so that they can all go on line 1, otherwise it would need to keep track
    # of which lines were skipped in order to figure out the line to put the next one on
    emitline "$1" '1s/^/wontreturn int metalend(int returnValue) {\n  printf("\\nReturn value was %d.\\n", returnValue);\n  for (;;) ;\n}\n/'
    emitline "$1" '1s/^/STATIC_YOINK("EfiMain");\n/' 'STATIC_YOINK("EfiMain");'
    emitline "$1" '1s/^/STATIC_YOINK("_idt");\n/' 'STATIC_YOINK("_idt");'
    emitline "$1" '1s/^/STATIC_YOINK("vga_console");\n/' 'STATIC_YOINK("vga_console");'
    # TODO put everything after includes and turn stdio include check back on
    emitline "$1" '1s/^/#include "libc\/stdio\/stdio.h"\n/' #'#include "libc[\/]stdio[\/]stdio.h"'
    #emitline "$1" '1s/^/#include "libc\/log\/log.h"\n/' '#include "libc[\/]log[\/]log.h"'

    # replace calls to exit and mbedtls_exit with a call to metalend
    # TODO replace exit calls in headers with metalend calls as well. I tried sticking a define at the top of the
    # file but that changes the original definition of exit to be called metalend in addition to the one above
    # fixing this would also remove the need for the mbedtls_exit replacement since it is just a macro for exit
    emitline "$1" 's/\([[:space:]]\{1,\}\)exit(/\1metalend(/'
    emitline "$1" 's/\([[:space:]]\{1,\}\)mbedtls_exit(/\1metalend(/'

    # TODO figure out why calling ShowCrashReports sometimes causes crashes
    #emitline "$1" '/int main/ a\  ShowCrashReports();' 'ShowCrashReports();'

    # based on https://unix.stackexchange.com/a/533803, replaces any returns in main with a call to metalend with the returned value.
    emitline "$1" '/main(.*{/,/^}/ s/return[[:space:]]\{1,\}\([[:alnum:]]\{1,\}\)[[:space:]]*;/metalend(\1);/'
    # in case the program doesn't have a return
    emitline "$1" '/main(.*{/,/^}/ s/^}/  metalend(0);\n}/'

    preppedSource=true
}

# $1: path to a cosmo c source file (including name)
# $2: name of the same cosmo c source file
restoresource() {
    [ ! -f "$2" ] || mv "$2" "$1"
}

# $1: path of the file to emit to
# $2: emitting sed pattern
# $3: optional sed pattern to check for first, only emits if it find nothing
emitline() {
    if [ -n "$3" ] ; then
        optionaltextline=$(sed -n '/'"$3"'/{=;q;}' "$1")
        if [ -n "$optionaltextline" ] ; then
            return
        fi
    fi

    sed "$2" "$1" > temp.c && mv temp.c "$1"
}


# $1: length of longest tested program's path relative to the cosmo git repo
writeresulttableheader() {
    if [ "$createResultsTable" = false ] ; then
        return
    fi

    baseProgramNameWidth=14 # length of " Program Name "
    requiredProgramNameWidth=$(($1 + 2))
    if [ "$requiredProgramNameWidth" -gt "$baseProgramNameWidth" ] ; then
        programNameWidth=$requiredProgramNameWidth
    else
        programNameWidth=$baseProgramNameWidth
    fi

    # +--------------+--------------+-------------+-------------+
    # | Program Name | Linux Result | Bios Result | Uefi Result |
    # +--------------+--------------+-------------+-------------+

    printf "+%""$programNameWidth""s+" | tr " " "-"         > cosmoboottesting.txt
    [ "$testLinux" = true ] && printf -- "--------------+" >> cosmoboottesting.txt
    [ "$testBios" = true ]  && printf -- "-------------+"  >> cosmoboottesting.txt
    [ "$testUefi" = true ]  && printf -- "-------------+"  >> cosmoboottesting.txt

    printf "\n| Program Name %$((programNameWidth - baseProgramNameWidth))s|" >> cosmoboottesting.txt
    [ "$testLinux" = true ] && printf " Linux Result |"                       >> cosmoboottesting.txt
    [ "$testBios" = true ]  && printf " Bios Result |"                        >> cosmoboottesting.txt
    [ "$testUefi" = true ]  && printf " Uefi Result |"                        >> cosmoboottesting.txt

    printf "\n+%""$programNameWidth""s+" | tr " " "-"      >> cosmoboottesting.txt
    [ "$testLinux" = true ] && printf -- "--------------+" >> cosmoboottesting.txt
    [ "$testBios" = true ]  && printf -- "-------------+"  >> cosmoboottesting.txt
    [ "$testUefi" = true ]  && printf -- "-------------+"  >> cosmoboottesting.txt
}

# $1: length of longest tested program's path relative to the cosmo git repo
# $2: path to a cosmo binary relative to the cosmo directory
# $3: linux result for that binary, a value must be provided
# $4: bios result for that binary, a value must be provided
# $5: uefi result for that binary, a value must be provided
writeresultentry() {
    baseProgramNameWidth=$((${#2} + 2))
    requiredProgramNameWidth=$(($1 + 2))
    if [ "$requiredProgramNameWidth" -gt "$baseProgramNameWidth" ] ; then
        programNameWidth=$requiredProgramNameWidth
    else
        programNameWidth=$baseProgramNameWidth
    fi

    # | $2           | $3           | $4          | $5          |
    # +--------------+--------------+-------------+-------------+

    printf "\n| %s %$((programNameWidth - baseProgramNameWidth))s|" "$2" >> cosmoboottesting.txt
    [ "$testLinux" = true ] && printf " %s%$((13 - ${#3}))s|" "$3"       >> cosmoboottesting.txt
    [ "$testBios" = true ]  && printf " %s%$((12 - ${#4}))s|" "$4"       >> cosmoboottesting.txt
    [ "$testUefi" = true ]  && printf " %s%$((12 - ${#5}))s|" "$5"       >> cosmoboottesting.txt

    printf "\n+%""$programNameWidth""s+" | tr " " "-"      >> cosmoboottesting.txt
    [ "$testLinux" = true ] && printf -- "--------------+" >> cosmoboottesting.txt
    [ "$testBios" = true ]  && printf -- "-------------+"  >> cosmoboottesting.txt
    [ "$testUefi" = true ]  && printf -- "-------------+"  >> cosmoboottesting.txt
}


# $1: return variable, based on https://stackoverflow.com/a/3243034
# $2: path to a cosmo binary relative to the current directory
# $3: name of that cosmo binary
testonlinux() {
    if [ "${skipListLinux#* "$3" }" != "$skipListLinux" ] ; then
        echo Skipping "$3" on linux
        eval "$1='Skipped'"
        return
    elif [ "$testLinux" = false ] ; then
        eval "$1='Skipped'"
        return
    fi

    ./createimage.sh "$cosmoFolder" "$sourceCosmoFilePath" linux

    printf "\nStarting %s on linux:\n\n" "$3"
    trap : INT
    $2
    trap 'restoresource $sourceFilePath $sourceFileName; exit' INT
    printf "\nReturn value was %d.\n" "$?"

    gettestresult "$1" "$3" linux
}

# $1: return variable, based on https://stackoverflow.com/a/3243034
# $2: path to a cosmo binary relative to the current directory
# $3: name of that cosmo binary
testonbios() {
    if [ "${skipListBios#* "$3" }" != "$skipListBios" ] ; then
        echo "Skipping $3 on qemu(bios)"
        eval "$1=Skipped"
        return
    elif [ "$testBios" = false ] ; then
        eval "$1=Skipped"
        return
    fi

    prepsource "$sourceFilePath" "$sourceFileName"
    ./createimage.sh "$cosmoFolder" "$sourceCosmoFilePath" bios

    printf "\nStarting %s on qemu(bios):\n\n" "$3"
    trap : INT
    if [ "$guiQemu" = true ] ; then
        qemu-system-x86_64 -drive format=raw,file="$3".bios.flp
    else
        qemu-system-x86_64 -drive format=raw,file="$3".bios.flp -nographic
    fi
    trap 'restoresource "$sourceFilePath" "$sourceFileName"; exit' INT
    printf "\n"

    gettestresult "$1" "$3" bios
}

# $1: return variable, based on https://stackoverflow.com/a/3243034
# $2: path to a cosmo binary relative to the current directory
# $3: name of that cosmo binary
testonuefi() {
    if [ "${skipListUefi#* "$3" }" != "$skipListUefi" ] ; then
        echo "Skipping $3 on qemu(uefi)"
        eval "$1=Skipped"
        return
    elif [ "$testUefi" = false ] ; then
        eval "$1=Skipped"
        return
    fi

    prepsource "$sourceFilePath" "$sourceFileName"
    ./createimage.sh "$cosmoFolder" "$sourceCosmoFilePath" uefi

    printf "\nStarting %s on qemu(uefi):\n\n" "$3"
    trap : INT
    if [ "$guiQemu" = true ] ; then
        qemu-system-x86_64 -bios OVMF.fd -drive format=raw,file="$3".uefi.flp,if=none,id=disk1 -device virtio-blk-pci,drive=disk1,bootindex=1
    else
        qemu-system-x86_64 -bios OVMF.fd -drive format=raw,file="$3".uefi.flp,if=none,id=disk1 -device virtio-blk-pci,drive=disk1,bootindex=1 -nographic
    fi
    trap 'restoresource "$sourceFilePath" "$sourceFileName"; exit' INT
    printf "\n"

    gettestresult "$1" "$3" uefi
}

# $1: return variable
# $2: name of a cosmo binary
# $3: target name
gettestresult() {
    if [ "$createResultsTable" = false ] ; then
        printf "Press any when finished with %s on %s." "$2" "$3"
        read -r ignored
        return
    fi

    # based on https://stackoverflow.com/a/226724
    while true ; do
        printf "Did %s work correctly on %s? (y/n/?): " "$2" "$3"
        read -r yn
        case $yn in
            [Yy]* ) eval "$1='Working'"; break;;
            [Nn]* ) eval "$1='Broken'"; break;;
            [?]*  ) eval "$1='Unknown'"; break;;
            * ) echo "Please answer yes or no.";;
        esac
    done
}

# $1: path to a cosmo binary relative to the cosmo git repo
runtest() {
    sourceFilePath=$1 # required for prepsource currently
    sourceCosmoFilePath=${1#"$cosmoFolder"/}
    sourceFileName=${1##*/}
    sourceBaseFileName=${sourceFileName%.c}

    linuxResult='Error'
    biosResult='Error'
    uefiResult='Error'
    preppedSource=false

    testonlinux linuxResult "$cosmoFolder"/o/"$subFolder"/"$sourceBaseFileName".com "$sourceBaseFileName"
    testonbios biosResult "$cosmoFolder"/o/"$subFolder"/"$sourceBaseFileName".com "$sourceBaseFileName"
    testonuefi uefiResult "$cosmoFolder"/o/"$subFolder"/"$sourceBaseFileName".com "$sourceBaseFileName"

    if [ "$testBios" = true ] || [ "$testUefi" = true ] ; then
        restoresource "$1" "$sourceFileName"
    fi

    writeresultentry "$longestFilePath" "$sourceCosmoFilePath" "$linuxResult" "$biosResult" "$uefiResult"
    ./createimage.sh clean
}


rm -rf ./*.c ./*.flp typescript hello.txt
./createimage.sh clean
if [ "$1" = clean ] ; then
    exit
fi

# always restore unmodified file on exit
trap 'restoresource "$sourceFilePath" "$sourceFileName"; exit' EXIT INT HUP TERM

# run single test if requested
if [ "$#" -eq 1 ] && [ -e "$cosmoFolder"/"$1" ] ; then
    createResultsTable=false
    runtest "$cosmoFolder"/"$1"
    exit 0
elif [ "$#" -gt 0 ] ; then
    exit 1
fi

# based on https://unix.stackexchange.com/a/478926
longestFilePath=$(cd "$cosmoFolder" && find "$subFolder" -type f | awk 'BEGIN { maxlength = 0 } length( $NF ) > maxlength { maxlength = length( $NF ) } END { print maxlength }')
writeresulttableheader "$longestFilePath"

for sourceFilePath in "$cosmoFolder"/"$subFolder"/*.c ; do
    [ -e "$sourceFilePath" ] || break
    runtest "$sourceFilePath"
done
