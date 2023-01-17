#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "third_party/make/getopt.h"

#include "third_party/xasm/file.h"
#include "third_party/xasm/instruction.h"

#define version_string "0.5.0"
#define help_string_format "X-TOY Assembler %s\n\
Usage: %s [general flags] -o path to output path to asm\n\
\n\
General Flags:\n\
-h, --help      Display this usage information and then quit.\n\
-v, --version   Display version history and then quit.\n\
Required Flags:\n\
-o, --output    Specify path for output binary.\n"
#define version_string_format "X-TOY Assembler %s\n\
Changelog:\n\
0.1.0   Added basic argument parsing and help info.\n\
0.2.0   Added long option and non option argument parsing.\n\
0.2.1   Fixed a crash when providing the path to the file to be assembled.\n\
0.3.0   Parse operation mnemonics from provided assembly file.\n\
0.3.1   Fixed a potential crash and ensure that using -h and -v do not return an error code.\n\
0.4.0   Parse operation arguments from provided assembly file.\n\
0.5.0   Output Visual X-TOY compatable binary.\n"

void PrintUsage(char* path)
{
    printf(help_string_format, version_string, path);
}

void PrintVersionInfo()
{
    printf(version_string_format, version_string);
}

bool ParseParameters(int argc, char* argv[], char** assemblyFilePath, char **binaryFilePath)
{
    int shortOptionValue = 0;
    int longOptionIndex = 0;

    char* shortOptions = ":hvo:";
    static struct option longOptions[] =
    {
        { "help",    no_argument,       NULL, 'h'  },
        { "version", no_argument,       NULL, 'v'  },
        { "output",  required_argument, NULL, 'o'  },
        { NULL,      no_argument,       NULL, '\0' }
    };

    if (argc == 1)
    {
        fprintf(stderr, "Error: No arguments provided.\n");
        return false;
    }

    *binaryFilePath = NULL;

    opterr = 0;
    while ((shortOptionValue = getopt_long(argc, argv, shortOptions, longOptions, &longOptionIndex)) != -1)
    {
        switch (shortOptionValue)
        {
            case 'h':
                PrintUsage(*argv);
                return true;
            case 'v':
                PrintVersionInfo(*argv);
                return true;
            case 'o':
                *binaryFilePath = optarg;
                break;
            case ':':
                fprintf(stderr, "Error: Missing argument for option '-%c'.\n", optopt);
                return false;
            case '?':
            default:
                if (isprint(optopt))
                {
                    fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
                }
                else
                {
                    fprintf(stderr, "Error: Unknown option character '\\x%x'.\n", optopt);
                }
                return false;
        }
    }

    if (optind == argc - 1)
    {
        // Not validating that this is a valid file at this point because of toctou
        *assemblyFilePath = argv[optind];
    }
    else
    {
        fprintf(stderr, "Error: No path to file to assemble was provided.\n");
        return false;
    }

    if (*binaryFilePath == NULL)
    {
        fprintf(stderr, "Error: Specifying an output file with '-o' is required.\n");
        return false;
    }

    return true;
}

int main(int argc, char* argv[])
{
    char* assemblyFilePath = NULL;
    char* binaryFilePath;
    bool succeeded = ParseParameters(argc, argv, &assemblyFilePath, &binaryFilePath);
    if (!succeeded || assemblyFilePath == NULL)
    {
        return (int)!succeeded;
    }

    uint_fast8_t operationCount;
    struct Operation operations[MAX_OPERATION_COUNT];
    succeeded = ParseFile(assemblyFilePath, &operationCount, operations);
    if (!succeeded)
    {
        return false;
    }

    succeeded = OutputBinary(binaryFilePath, operationCount, operations);

    return (int)!succeeded;
}
