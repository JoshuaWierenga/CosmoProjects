#include "libc/stdio/stdio.h"
#include "third_party/getopt/getopt.h"

#define version_string "0.1"
#define help_string_format "X-TOY Assembler %s\n\
Usage: %s [general flags] [path to asm]\n\
\n\
General Flags:\n\
-?, -h: Display this usage information and then quit.\n\
-v: Display version history and then quit.\n"
#define version_string_format "X-TOY Assembler %s\n\
Changelog:\n\
0.1 Added basic argument parsing and help info.\n"

void PrintUsage(char* path)
{
	printf(help_string_format, version_string, path);
}

void PrintVersionInfo()
{
	printf(version_string_format, version_string);
}

bool ParseParameters(int argc, char* argv[])
{
	int opt;
	while ((opt = getopt(argc, argv, "hv")) != -1)
	{
		switch (opt)
		{
			case 'v':
				PrintVersionInfo(*argv);
				return false;
			case 'h':
			default:
				PrintUsage(*argv);
				return false;
		}
	}

	return false;
}


int main(int argc, char* argv[])
{
	ParseParameters(argc, argv);
	return 0;
}
