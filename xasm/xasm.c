#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "third_party/make/getopt.h"

#define version_string "0.2"
#define help_string_format "X-TOY Assembler %s\n\
Usage: %s [general flags] path to asm\n\
\n\
General Flags:\n\
-h, --help      Display this usage information and then quit.\n\
-v, --version   Display version history and then quit.\n"
#define version_string_format "X-TOY Assembler %s\n\
Changelog:\n\
0.1     Added basic argument parsing and help info.\n\
0.2     Added long option and non option argument parsing.\n\
0.2.1   Fixed a crash when providing the path to the file to be assembled.\n"

void PrintUsage(char* path)
{
    printf(help_string_format, version_string, path);
}

void PrintVersionInfo()
{
    printf(version_string_format, version_string);
}

bool ParseParameters(int argc, char* argv[], char** assemblyFilePath)
{
    int shortOptionValue = 0;
    int longOptionIndex = 0;

    char* shortOptions = ":hv";
    static struct option longOptions[] =
    {
        { "help",    no_argument, NULL, 'h'  },
        { "version", no_argument, NULL, 'v'  },
        { NULL,      0,           NULL, '\0' }
    };

    if (argc == 1)
    {
        fprintf(stderr, "Error: No arguments provided.\n");
        return false;
    }

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
            case '?':
                if (isprint(optopt))
                {
                    fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
                }
                else
                {
                    fprintf(stderr, "Error: Unknown option character '\\x%x'.\n", optopt);
                }
                return false;
            default:
                fprintf(stderr, "Error: Unknown option character '\\x%x'.\n", optopt);
                return false;
        }
    }

    *assemblyFilePath = NULL;
    if (optind == argc - 1)
    {
        // Not validating that this is a valid file at this point because of toctou
        *assemblyFilePath = argv[optind];
    }

    return *assemblyFilePath != NULL;
}


int main(int argc, char* argv[])
{
    char* assemblyFilePath = NULL;
    bool errorOccurred = ParseParameters(argc, argv, &assemblyFilePath);

    if (assemblyFilePath != NULL)
    {
        printf("Assembly file path: %s\n", assemblyFilePath);
    }

    return (int)!errorOccurred;
}
