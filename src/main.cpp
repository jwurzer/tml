#include <tml/tml_parser.h>
#include <cfg/cfg.h>
#include <cfg/cfg_string.h>

#include <string>
#include <iostream>

namespace
{
	void printHelp(const char* appname)
	{
		std::cerr << "\n" <<
				appname << " <command> [<args>]\n" <<
				"\n" <<
				"commands and args:\n" <<
				"------------------\n" <<
				"  help                     ... print this help\n" <<
				"  print <filename>         ... print the tml file\n" <<
				std::endl;
	}

	int printTml(const char* filename)
	{
		cfg::TmlParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp, true /* inclEmptyLines */,
				true /* inclComments */)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		std::string s = cfg::cfgstring::nameValuePairToString(0, cvp);
		std::cout << s << std::endl;
		return 0;
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printHelp(argv[0]);
		return 1;
	}
	std::string command = argv[1];
	if (command == "help") {
		if (argc != 2) {
			std::cerr << "help command doesn't support any arguments" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		printHelp(argv[0]);
		return 0;
	}
	if (command == "print") {
		if (argc != 3) {
			std::cerr << "print command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return printTml(argv[2]);
	}
	std::cerr << "command '" << command << "' is not supported" << std::endl;
	printHelp(argv[0]);
	return 1;
}
