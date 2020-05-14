#include <tml/tml_parser.h>
#include <cfg/cfg.h>
#include <cfg/cfg_string.h>
#include <cfg/cfg_template.h>
#include <tml/tml_string.h>

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
				"  help                        ... print this help\n" <<
				"  print <filename>            ... print the tml file\n" <<
				"  print-values <filename>     ... print the tml file without empty lines and comments\n" <<
				"  print-tml <filename>        ... print the tml file in tml-format\n" <<
				"  print-tml-values <filename> ... print the tml file without empty lines and comments in tml-format\n" <<
				"  templates <filename>        ... load and print templates from tml file\n" <<
				std::endl;
	}

	int printTml(const char* filename, bool inclEmptyLines, bool inclComments)
	{
		cfg::TmlParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp, inclEmptyLines, inclComments)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		std::string s = cfg::cfgstring::nameValuePairToString(0, cvp);
		std::cout << s << std::endl;
		return 0;
	}

	int printTmlAsTml(const char* filename, bool inclEmptyLines, bool inclComments)
	{
		cfg::TmlParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp, inclEmptyLines, inclComments)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		//std::string s = cfg::tmlstring::nameValuePairToString(0, cvp);
		std::string s = cfg::tmlstring::valueToString(0, cvp.mValue);
		std::cout << s << std::endl;
		return 0;
	}

	int loadAndPrintTemplates(const char* filename)
	{
		cfg::TmlParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp, true, true)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		cfg::cfgtemp::TemplateMap templateMap;
		std::string outErrorMsg;
		if (!cfg::cfgtemp::addTemplates(templateMap, cvp.mValue,
				true /* removeTemplatesFromCfgValue */,
				"template",
				outErrorMsg)) {
			std::cerr << "error: " << outErrorMsg << std::endl;
			return 1;
		}
		std::cout << "templates: " << templateMap.size() << std::endl;
		for (const auto& temp : templateMap) {
			std::cout << temp.second.toString();
		}
		std::string s = cfg::tmlstring::valueToString(0, cvp.mValue);
		std::cout << "============================================" << std::endl;
		std::cout << s << std::endl;
		std::cout << "============================================" << std::endl;
		outErrorMsg.clear();
		if (!cfg::cfgtemp::useTemplates(templateMap, cvp.mValue, "use-template", outErrorMsg)) {
			std::cerr << "error: " << outErrorMsg << std::endl;
			return 1;
		}
		s = cfg::tmlstring::valueToString(0, cvp.mValue);
		std::cout << "============================================" << std::endl;
		std::cout << s << std::endl;
		std::cout << "============================================" << std::endl;
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
		return printTml(argv[2], true, true);
	}
	if (command == "print-values") {
		if (argc != 3) {
			std::cerr << "print-values command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return printTml(argv[2], false, false);
	}
	if (command == "print-tml") {
		if (argc != 3) {
			std::cerr << "print-tml command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return printTmlAsTml(argv[2], true, true);
	}
	if (command == "print-tml-values") {
		if (argc != 3) {
			std::cerr << "print-tml-values command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return printTmlAsTml(argv[2], false, false);
	}
	if (command == "templates") {
		if (argc != 3) {
			std::cerr << "templates command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return loadAndPrintTemplates(argv[2]);
	}
	std::cerr << "command '" << command << "' is not supported" << std::endl;
	printHelp(argv[0]);
	return 1;
}
