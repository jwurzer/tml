#include <tml/tml_parser.h>
#include <cfg/cfg.h>
#include <cfg/cfg_string.h>
#include <cfg/cfg_template.h>
#include <cfg/cfg_translation.h>
#include <cfg/cfg_include.h>
#include <cfg/cfg_cppstring.h>
#include <tml/tml_string.h>
#include <tml/tml_file_loader.h>
#include <json/json_string.h>
#include <json/json_parser.h>
#include <cfg_cppstring_example.h>

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
				"  help                         ... print this help\n" <<
				"  print <filename>             ... print the tml file\n" <<
				"  print-values <filename>      ... print the tml file without empty lines and comments\n" <<
				"  print-tml <filename>         ... print the tml file in tml-format\n" <<
				"  print-tml-values <filename>  ... print the tml file without empty lines and comments in tml-format\n" <<
				"  print-tml-stdin              ... print the tml from stdin in tml-format\n" <<
				"  templates <filename>         ... load and print templates from tml file\n" <<
				"  translations <filename>      ... load and print translations from tml file\n" <<
				"  translations <filename> <prefix>   ... load and print translations from tml file\n" <<
				"  variables <filename>         ... load and print variables from tml file\n" <<
				"  include <filename>           ... load tml file and include all other tml files and print it\n" <<
				"  include-buf <filename>       ... load tml file and include all other tml files and print it (with file buffering)\n" <<
				"  print-tml-entries <filename> ... print each tml entry per line\n" <<
				"  printjson <filename>         ... print the json file\n" <<
				"  printjson2tml <filename>     ... print the json file as tml\n" <<
				"  printtml2json <filename>     ... print the tml file as json\n" <<
				"  printjson2json <filename>    ... print the json file as json\n" <<
				"  printtml2cpp <filename>      ... print the tml file as cpp\n" <<
				"  printcppexample              ... print the cpp example\n" <<
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

	int printTmlAsTmlFromStdIn(bool inclEmptyLines, bool inclComments)
	{
		std::string line;
		std::string lines;
		std::cout << "========================================" << std::endl;
		std::cout << "Please input tml. Quit input with CTRL+D" << std::endl;
		std::cout << "========================================" << std::endl;
		while (std::getline(std::cin, line))
		{
			lines += line + "\n";
		}
		std::cout << "input:" << std::endl;
		std::cout << "======" << std::endl;
		std::cout << lines << std::endl;
		std::cout << "======" << std::endl;
		cfg::TmlParser p;
		p.setStringBuffer("standard-input", lines);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp, inclEmptyLines, inclComments)) {
			std::cerr << "parse from standard input failed" << std::endl;
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

	int loadAndPrintTranslations(const char* filename, const std::string& prefix)
	{
		cfg::TmlParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp, true, true)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		cfg::Value& value = cvp.mValue;
		std::string errorMsg;
		cfg::cfgtr::LanguageMap languageMap;
		if (!cfg::cfgtr::addTranslations(languageMap, value, true,
				"translations", errorMsg)) {
			std::cout << errorMsg << std::endl;
			return false;
		}
		if (!languageMap.empty()) {
			std::cout << "Language prefixes:";
			for (const auto& langEntry : languageMap) {
				std::cout << " '" << langEntry.first << "'";
			}
			std::cout << std::endl;
		}
		else {
			std::cout << "Language prefixes: none" << std::endl;
		}

		for (const auto& langEntry : languageMap) {
			std::cout << "Language '" << langEntry.first << "':" << std::endl;
			for (const auto& translationEntry : langEntry.second) {
				std::cout << "\t" << translationEntry.first << " = " <<
						cfg::tmlstring::valueToString(0, translationEntry.second.mValue);
			}
		}

		std::string langPrefix = prefix;
		if (langPrefix.empty() && !languageMap.empty()) {
			langPrefix = languageMap.begin()->first;
			std::cout << "No prefix specified. Using first prefix '" << langPrefix << "'" << std::endl;
		}

		if (!cfg::cfgtr::useTranslations(languageMap,
				langPrefix, "tr(", value, errorMsg)) {
			std::cout << errorMsg << " for language: " << langPrefix << std::endl;
			return false;
		}

		std::cout << "==================================" << std::endl;
		std::cout << cfg::tmlstring::valueToString(0, value);
		return 0;
	}

	int loadAndPrintVariables(const char* filename)
	{
		cfg::TmlParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp, true, true)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		cfg::Value& value = cvp.mValue;
		std::string errorMsg;
		cfg::cfgtr::LanguageMap languageMap;
		if (!cfg::cfgtr::addVariables(languageMap, value, true,
				"variables", errorMsg)) {
			std::cout << errorMsg << std::endl;
			return false;
		}
		if (languageMap.empty()) {
			std::cout << "No variables exist" << std::endl;
		}
		else if (languageMap.size() > 1) {
			std::cout << "Language prefixes:";
			for (const auto& langEntry : languageMap) {
				std::cout << " '" << langEntry.first << "'";
			}
			std::cout << std::endl;
		}
		else if (languageMap.begin()->first != "") {
			std::cout << "Wrong prefix variables! Prefix is '" <<
					languageMap.begin()->first << "'" << std::endl;
		}

		for (const auto& langEntry : languageMap) {
			std::cout << "variables:" << std::endl;
			for (const auto& translationEntry : langEntry.second) {
				std::cout << "\t" << translationEntry.first << " = " <<
						cfg::tmlstring::valueToString(0, translationEntry.second.mValue);
			}
		}

		if (!cfg::cfgtr::useTranslations(languageMap,
				"", "$(", value, errorMsg)) {
			std::cout << errorMsg << " for variables" << std::endl;
			return false;
		}

		std::cout << "==================================" << std::endl;
		std::cout << cfg::tmlstring::valueToString(0, value);
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

	int printTmlEntries(const char* filename)
	{
		cfg::TmlParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.begin()) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		int rv = -3;
		for (;;) {
			std::string line;
			int lineNumber = 0;
			rv = p.getNextTmlEntry(cvp, &line, &lineNumber);

			std::cout << lineNumber << ":\t'" << line << "'" << std::endl;
			std::cout << cfg::cfgstring::nameValuePairToString(rv >= 0 ? rv : 0, cvp) << std::endl;
			if (rv < 0) {
				break;
			}
		}
		std::cout << "last return value: " << rv << std::endl;
		std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
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

	int printTmlToCpp(const char* filename, bool inclEmptyLines, bool inclComments)
	{
		cfg::TmlParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp, inclEmptyLines, inclComments)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		std::string s = cfg::cppstring::valueToString(0, cvp.mValue, false);
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
	if (command == "print-tml-stdin") {
		if (argc != 2) {
			std::cerr << "print-tml-stdin command need no arguments" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return printTmlAsTmlFromStdIn(true, true);
	}
	if (command == "templates") {
		if (argc != 3) {
			std::cerr << "templates command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return loadAndPrintTemplates(argv[2]);
	}
	if (command == "translations") {
		if (argc != 3 && argc != 4) {
			std::cerr << "translations command need one or two arguments (filename or filename + prefix)" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return loadAndPrintTranslations(argv[2], (argc == 4) ? argv[3] : "");
	}
	if (command == "variables") {
		if (argc != 3) {
			std::cerr << "variables command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return loadAndPrintVariables(argv[2]);
	}
	if (command == "include" || command == "include-buf") {
		if (argc != 3) {
			std::cerr << "include command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return includeAndPrint(argv[2], true, true, command == "include-buf", false);
	}
	if (command == "print-tml-entries") {
		if (argc != 3) {
			std::cerr << "print-tml-entries command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return printTmlEntries(argv[2]);
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
	if (command == "printtml2cpp") {
		if (argc != 3) {
			std::cerr << "printtml2cpp command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return printTmlToCpp(argv[2], true, true);
	}
	if (command == "printcppexample") {
		std::string s = cfg::cppstring::valueToString(0, example, false);
		std::cout << s << std::endl;
		return 0;
	}
	std::cerr << "command '" << command << "' is not supported" << std::endl;
	printHelp(argv[0]);
	return 1;
}
