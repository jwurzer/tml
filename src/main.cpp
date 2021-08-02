#include <tml/tml_parser.h>
#include <cfg/cfg.h>
#include <cfg/cfg_string.h>
#include <cfg/cfg_template.h>
#include <cfg/cfg_include.h>
#include <tml/tml_string.h>
#include <tml/tml_file_loader.h>
#include <json/json_string.h>
#include <json/json_parser.h>

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
				"  include <filename>          ... load tml file and include all other tml files and print it\n" <<
				"  include-buf <filename>      ... load tml file and include all other tml files and print it (with file buffering)\n" <<
				"  printjson <filename>        ... print the json file\n" <<
				"  printjson2tml <filename>    ... print the json file as tml\n" <<
				"  printtml2json <filename>    ... print the tml file as json\n" <<
				"  printjson2json <filename>   ... print the json file as json\n" <<
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

	int includeAndPrint(const char* filename, bool inclEmptyLines,
			bool inclComments, bool withFileBuffering, bool forceDeepByStoredDeepValue)
	{
		cfg::TmlFileLoader loader;
		cfg::Value value;
		std::string outErrorMsg;
		cfg::inc::TFileMap includedFiles;
		if (!cfg::inc::loadAndIncludeFiles(value, includedFiles, filename, loader,
				"include", inclEmptyLines, inclComments, withFileBuffering,
				outErrorMsg)) {
			std::cerr << "parse/includes for " << filename << " failed" << std::endl;
			std::cerr << "error: " << outErrorMsg << std::endl;
			return 1;
		}

#if 0
		{
			std::string s = cfg::cfgstring::valueToString(0, value);
			std::cout << s << std::endl;
		}
#endif

		{
			std::string s = cfg::tmlstring::valueToString(0, value, forceDeepByStoredDeepValue);
			std::cout << s << std::endl;
		}

		std::cout << "*** included files: ***" << std::endl;
		for (const auto& v : includedFiles) {
			std::cout << v.first << ", count: " << v.second << std::endl;
		}
		return 0;
	}

	int printJson(const char* filename)
	{
		cfg::JsonParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		std::string s = cfg::cfgstring::nameValuePairToString(0, cvp);
		std::cout << s << std::endl;
		return 0;
	}

	int printJsonToTml(const char* filename)
	{
		cfg::JsonParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		std::string s = cfg::tmlstring::valueToString(0, cvp.mValue);
		std::cout << s << std::endl;
		return 0;
	}

	int printTmlToJson(const char* filename, bool inclEmptyLines, bool inclComments)
	{
		cfg::TmlParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp, inclEmptyLines, inclComments)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		//std::string s = cfg::jsonstring::nameValuePairToString(0, cvp, -1);
		std::string s = cfg::jsonstring::valueToString(0, cvp.mValue, 0);
		std::cout << s << std::endl;
		return 0;
	}

	int printJsonToJson(const char* filename)
	{
		cfg::JsonParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		//std::string s = cfg::jsonstring::nameValuePairToString(0, cvp, -1);
		std::string s = cfg::jsonstring::valueToString(0, cvp.mValue, 0);
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
	if (command == "include" || command == "include-buf") {
		if (argc != 3) {
			std::cerr << "include command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return includeAndPrint(argv[2], true, true, command == "include-buf", false);
	}
	if (command == "printjson") {
		if (argc != 3) {
			std::cerr << "printjson command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return printJson(argv[2]);
	}
	if (command == "printjson2tml") {
		if (argc != 3) {
			std::cerr << "printjson2tml command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return printJsonToTml(argv[2]);
	}
	if (command == "printtml2json") {
		if (argc != 3) {
			std::cerr << "printtml2json command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return printTmlToJson(argv[2], true, true);
	}
	if (command == "printjson2json") {
		if (argc != 3) {
			std::cerr << "printjson2json command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return printJsonToJson(argv[2]);
	}
	std::cerr << "command '" << command << "' is not supported" << std::endl;
	printHelp(argv[0]);
	return 1;
}
