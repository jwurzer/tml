#include <tml/tml_parser.h>
#include <cfg/cfg.h>
#include <cfg/cfg_string.h>
#include <cfg/cfg_template.h>
#include <cfg/cfg_translation.h>
#include <cfg/cfg_include.h>
#include <cfg/cfg_cppstring.h>
#include <cfg/cfg_schema.h>
#include <cfg/parser_file_loader.h>
#include <tml/tml_string.h>
#include <json/json_string.h>
#include <json/json_parser.h>
#include <cfg_cppstring_example.h>
#include <interpreter/interpreter.h>
#include <interpreter/interpreter_unit_tests.h>
#include <btml/btml_stream.h>

#include <string>
#include <iostream>
#include <chrono>

#define INCLUDE_UNIT_TESTS

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
				"  load-tml <filename>          ... check if loading the tml file is successful (no print)\n" <<
				"  load-btml <filename>         ... check if loading the btml file is successful (no print)\n" <<
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
				"  include-once <filename>      ... load tml file and include all other tml files and print it (only once)\n" <<
				"  include-once-buf <filename>  ... load tml file and include all other tml files and print it (only once, with file buffering)\n" <<
				"  print-tml-entries <filename> ... print each tml entry per line\n" <<
				"  printjson <filename>         ... print the json file\n" <<
				"  printjson2tml <filename>     ... print the json file as tml\n" <<
				"  printtml2json <filename>     ... print the tml file as json\n" <<
				"  printjson2json <filename>    ... print the json file as json\n" <<
				"  printtml2cpp <filename>      ... print the tml file as cpp\n" <<
				"  tml2btml all|shrink|strip|strip-shrink|afss <in-tml> <out-btml> ... convert a tml file to a btml file\n" <<
				"  btml2tml all <in-btml> <out-tml> ... convert a btml file to a tml file\n" <<
				//"  btml2tml all|strip <in-btml> <out-tml> ... convert a btml file to a tml file\n" <<
				"  printcppexample              ... print the cpp example\n" <<
				"  interpreter-tests            ... some unit-tests for interpreter\n" <<
				"  interpret <filename>         ... evaluate expressions\n" <<
				"  all-features <in-file> [<out-file>]     ... includes, templates, translations, profiles, variables, expressions\n" <<
				"  validate <schema-filename> <filename>   ... validate\n" <<
#ifdef INCLUDE_UNIT_TESTS
				"  unit-tests                   ... unit-tests\n" <<
#endif
				std::endl;
	}

	int onlyLoadTml(const char* filename, bool inclEmptyLines, bool inclComments)
	{
		auto start = std::chrono::steady_clock::now();
		cfg::TmlParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp, inclEmptyLines, inclComments)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		auto endParse = std::chrono::steady_clock::now();
		std::cout << "Total: " << std::chrono::duration_cast<std::chrono::milliseconds>(endParse - start).count() << "ms" << std::endl;
		return 0;
	}

	int onlyLoadBtml(const char* filename)
	{
		auto start = std::chrono::steady_clock::now();
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if (!ifs.is_open() || ifs.fail()) {
			std::cout << "Can't open " << filename << std::endl;
			return 1;
		}
		ifs.seekg(0, std::ios::end);
		std::size_t size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		std::vector<uint8_t> buf;
		buf.resize(size);
		ifs.read(reinterpret_cast<char*>(buf.data()), size);
		if (ifs.fail()) {
			std::cout << "Can't read full file content of " << filename << std::endl;
			return 1;
		}
		auto endFile = std::chrono::steady_clock::now();
		cfg::Value cfgValue;
		std::string errMsg;
		bool headerExist = false;
		bool stringTableExist = false;
		unsigned int stringTableEntryCount = false;
		unsigned int stringTableSize = false;

		unsigned int bytes = cfg::btmlstream::streamToValueWithOptionalHeader(
				buf.data(), static_cast<unsigned int>(buf.size()), cfgValue,
				&errMsg, headerExist, stringTableExist, stringTableEntryCount,
				stringTableSize);
		auto endConvert = std::chrono::steady_clock::now();

		if (bytes != buf.size()) {
			std::cerr << "Convert btml " << filename << " to cfg::Value failed. (" <<
					bytes << " != " << buf.size() << "), err: " <<
					errMsg << std::endl;
			return 1;
		}
		std::cout << "File loading: " << std::chrono::duration_cast<std::chrono::milliseconds>(endFile - start).count() << "ms" << std::endl;
		std::cout << "Convert from btml to cfg::Value: " << std::chrono::duration_cast<std::chrono::milliseconds>(endConvert - endFile).count() << "ms" << std::endl;
		std::cout << "Total: " << std::chrono::duration_cast<std::chrono::milliseconds>(endConvert - start).count() << "ms" << std::endl;
		if (!errMsg.empty()) {
			std::cout << "err msg: " << errMsg << std::endl;
		}
		std::cout << "header: " << (headerExist ? "yes" : "no") << std::endl;
		std::cout << "string table: " << (stringTableExist ? "yes" : "no") << std::endl;
		if (stringTableEntryCount > 0 || stringTableExist) {
			std::cout << "string table entries: " << stringTableEntryCount << std::endl;
		}
		if (stringTableSize > 0 || stringTableExist) {
			std::cout << "string table size: " << stringTableSize << std::endl;
		}
		return 0;
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
		// valueToString() always includes a \n. also for the last line.
		std::string s = cfg::tmlstring::valueToString(0, cvp.mValue);
		std::cout << s; // no std::endl here!
		//std::cout << std::endl; // not necessary. already include a \n.
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
		if (!cfg::cfgtemp::useTemplates(templateMap, cvp.mValue, "use-template", true, false, outErrorMsg)) {
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
				langPrefix, "tr(", true, value, errorMsg)) {
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
				"", "$(", true, value, errorMsg)) {
			std::cout << errorMsg << " for variables" << std::endl;
			return false;
		}

		std::cout << "==================================" << std::endl;
		std::cout << cfg::tmlstring::valueToString(0, value);
		return 0;
	}

	int includeAndPrint(const char* filename, bool includeOnce, bool inclEmptyLines,
			bool inclComments, bool withFileBuffering, bool forceDeepByStoredDeepValue)
	{
		cfg::ParserFileLoader loader(std::unique_ptr<cfg::TmlParser>(new cfg::TmlParser()));
		cfg::Value value;
		std::string outErrorMsg;
		cfg::inc::TFileMap includedFiles;
		if (!cfg::inc::loadAndIncludeFiles(value, includedFiles, filename, loader,
				"include", includeOnce, inclEmptyLines, inclComments, withFileBuffering,
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

	int allFeaturesToCfgValue(const char* filename, bool printAfterEachStep,
			cfg::Value& value, bool inclEmptyLines, bool inclComments);

	int convertTmlToBtml(const char* inTmlFilename, const char* outBtmlFilename,
			bool allFeatures, bool inclEmptyAndComment, bool shrinkBtml)
	{
		cfg::NameValuePair cvp;
		if (allFeatures) {
			int rv = allFeaturesToCfgValue(inTmlFilename, false,
					cvp.mValue, inclEmptyAndComment, inclEmptyAndComment);
			if (rv) {
				return rv;
			}
		}
		else {
			cfg::TmlParser p(inTmlFilename);
			if (!p.getAsTree(cvp, inclEmptyAndComment, inclEmptyAndComment)) {
				std::cerr << "parse " << inTmlFilename << " failed" << std::endl;
				std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
				return 1;
			}
		}
		std::vector<uint8_t> btml;
		unsigned int btmlLen = cfg::btmlstream::valueToStreamWithHeader(
				cvp.mValue, btml, shrinkBtml);
		if (btmlLen != btml.size()) {
			std::cerr << "convert " << inTmlFilename << " to btml failed (" <<
					btmlLen << " != " << btml.size() << ")" << std::endl;
			return 1;
		}
		std::ofstream fout(outBtmlFilename);
		fout.write(reinterpret_cast<const char*>(btml.data()), btml.size());
		if (fout.fail()) {
			std::cerr << "Create/write " << outBtmlFilename << " failed" << std::endl;
			return 1;
		}
		return 0;
	}

	int convertBtml2Tml(const char* inBtmlFilename, const char* outTmlFilename)
	{
		std::ifstream ifs(inBtmlFilename, std::ios::in | std::ios::binary);
		if (!ifs.is_open() || ifs.fail()) {
			std::cout << "Can't open " << inBtmlFilename << std::endl;
			return 1;
		}
		ifs.seekg(0, std::ios::end);
		std::size_t size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		std::vector<uint8_t> buf;
		buf.resize(size);
		ifs.read(reinterpret_cast<char*>(buf.data()), size);
		if (ifs.fail()) {
			std::cout << "Can't read full file content of " << inBtmlFilename << std::endl;
			return 1;
		}
		cfg::Value cfgValue;
		std::string errMsg;
		bool headerExist = false;
		bool stringTableExist = false;
		unsigned int stringTableEntryCount = 0;
		unsigned int stringTableSize = 0;

		unsigned int bytes = cfg::btmlstream::streamToValueWithOptionalHeader(
				buf.data(), static_cast<unsigned int>(buf.size()), cfgValue,
				&errMsg, headerExist, stringTableExist, stringTableEntryCount,
				stringTableSize);

		if (bytes != buf.size()) {
			std::cerr << "Convert btml " << inBtmlFilename << " to cfg::Value failed. (" <<
					bytes << " != " << buf.size() << "), err: " <<
					errMsg << std::endl;
			return 1;
		}
		if (!errMsg.empty()) {
			std::cout << "err msg: " << errMsg << std::endl;
		}
		std::cout << "header: " << (headerExist ? "yes" : "no") << std::endl;
		std::cout << "string table: " << (stringTableExist ? "yes" : "no") << std::endl;
		if (stringTableEntryCount > 0 || stringTableExist) {
			std::cout << "string table entries: " << stringTableEntryCount << std::endl;
		}
		if (stringTableSize > 0 || stringTableExist) {
			std::cout << "string table size: " << stringTableSize << std::endl;
		}
		std::ofstream ofs(outTmlFilename);
		if (!ofs.is_open()) {
			std::cout << "Can't open/create " << outTmlFilename << std::endl;
			return 1;
		}
		cfg::tmlstring::valueToStream(0, cfgValue, ofs);
		if (ofs.fail()) {
			std::cout << "Write to " << outTmlFilename << " failed." << std::endl;
			return 1;
		}
		ofs.close();
		return 0;
	}

	int interpret(const char* filename)
	{
		cfg::TmlParser p(filename);
		cfg::NameValuePair cvp;
		if (!p.getAsTree(cvp, true, true)) {
			std::cerr << "parse " << filename << " failed" << std::endl;
			std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		cfg::Value& value = cvp.mValue;
		std::cout << "==================== original ====================" << std::endl;
		std::string s = cfg::tmlstring::valueToString(0, value);
		std::cout << s << std::endl;

		std::stringstream errMsg;
		if (cfg::interpreter::interpretAndReplace(value, false, true, true, true, errMsg) == -1) {
			std::cout << "=========== evaluate expressions FAILED ==========" << std::endl;
			std::cout << errMsg.str() << std::endl;
			return 1;
		}
		std::cout << "=========== after evaluate expressions ===========" << std::endl;
		s = cfg::tmlstring::valueToString(0, value);
		std::cout << s << std::endl;
		return 0;
	}

	int allFeaturesToCfgValue(const char* filename, bool printAfterEachStep,
			cfg::Value& value, bool inclEmptyLines, bool inclComments)
	{
		if (printAfterEachStep) {
			cfg::TmlParser p(filename);
			cfg::NameValuePair cvp;
			if (!p.getAsTree(cvp, inclEmptyLines, inclComments)) {
				std::cerr << "parse " << filename << " failed" << std::endl;
				std::cerr << "error: " << p.getExtendedErrorMsg() << std::endl;
				return 1;
			}
			cfg::Value& v = cvp.mValue;
			std::cout << "==================== original ====================" << std::endl;
			std::string s = cfg::tmlstring::valueToString(0, v);
			std::cout << s << std::endl;
		}

		cfg::ParserFileLoader loader(std::unique_ptr<cfg::TmlParser>(new cfg::TmlParser()));
		value.clear();
		std::string outErrorMsg;
		cfg::inc::TFileMap includedFiles;
		bool includeOnce = true;
		bool withFileBuffering = true;
		if (!cfg::inc::loadAndIncludeFiles(value, includedFiles, filename, loader,
				"include", includeOnce, inclEmptyLines, inclComments, withFileBuffering,
				outErrorMsg)) {
			std::cerr << "parse/includes for " << filename << " failed" << std::endl;
			std::cerr << "error: " << outErrorMsg << std::endl;
			return 1;
		}

		if (printAfterEachStep) {
			std::cout << "==================== with includes ====================" << std::endl;
			std::string s = cfg::tmlstring::valueToString(0, value);
			std::cout << s << std::endl;
		}

		{
			cfg::cfgtemp::TemplateMap templateMap;
			std::string errorMsg;
			if (!cfg::cfgtemp::addTemplates(templateMap, value, true, "template",
					errorMsg)) {
				std::cout << "============ add templates FAILED ===========" << std::endl;
				std::cout << errorMsg << std::endl;
				return 1;
			}
			if (!cfg::cfgtemp::useTemplates(templateMap, value,
					"use-template", true, false, errorMsg)) {
				std::cout << "============ use templates FAILED ===========" << std::endl;
				std::cout << errorMsg << std::endl;
				return 1;
			}
			if (printAfterEachStep) {
				std::cout << "============ after apply templates ============" << std::endl;
				std::string s = cfg::tmlstring::valueToString(0, value);
				std::cout << s << std::endl;
			}
		}

		{
			std::string errorMsg;
			std::string languageId; // empty --> applyTranslations() uses the first available one
			//std::string languageId = "EN";
			if (!cfg::cfgtr::applyTranslations(value, "translations",
					"tr(", languageId, errorMsg)) {
				std::cout << "============ apply translations FAILED ===========" << std::endl;
				std::cout << errorMsg << std::endl;
				return 1;
			}
			if (printAfterEachStep) {
				std::cout << "============ after apply translations ============" << std::endl;
				std::string s = cfg::tmlstring::valueToString(0, value);
				std::cout << s << std::endl;
			}
		}

		{
			std::string errorMsg;
			std::string profileId; // empty --> applyTranslations() uses the first available one
			//std::string profileId = "P1";
			cfg::cfgtr::LanguageMap languageMap;
			if (!cfg::cfgtr::applyTranslations(value, "profiles",
					"pr(", profileId, errorMsg)) {
				std::cout << "============ apply translations for profiles FAILED ===========" << std::endl;
				std::cout << errorMsg << std::endl;
				return 1;
			}
			if (printAfterEachStep) {
				std::cout << "============ after apply profiles ============" << std::endl;
				std::string s = cfg::tmlstring::valueToString(0, value);
				std::cout << s << std::endl;
			}
		}

		{
			std::string errorMsg;
			if (!cfg::cfgtr::applyVariables(value, "variables",
					"$(", errorMsg)) {
				std::cout << "============= apply variables FAILED =============" << std::endl;
				std::cout << errorMsg << std::endl;
				return 1;
			}
			if (printAfterEachStep) {
				std::cout << "============== after apply variables =============" << std::endl;
				std::string s = cfg::tmlstring::valueToString(0, value);
				std::cout << s << std::endl;
			}
		}

		std::stringstream errMsg;
		if (cfg::interpreter::interpretAndReplace(value, false, true, true, true, errMsg) == -1) {
			std::cout << "=========== evaluate expressions FAILED ==========" << std::endl;
			std::cout << errMsg.str() << std::endl;
			return 1;
		}
		if (printAfterEachStep) {
			std::cout << "=========== after evaluate expressions ===========" << std::endl;
			std::string s = cfg::tmlstring::valueToString(0, value);
			std::cout << s << std::endl;
			std::cout << "=========== EOF ===========" << std::endl;
		}
		return 0;
	}

	int allFeatures(const char* filename, bool printAfterEachStep, const char* outputFilename)
	{
		cfg::Value value;
		int rv = allFeaturesToCfgValue(filename, printAfterEachStep, value, false, false);
		if (rv) {
			return rv;
		}
		if (outputFilename) {
			std::ofstream file(outputFilename);
			std::string s = cfg::tmlstring::valueToString(0, value);
			file << s;
			file.close();
		}
		return 0;
	}

	int validate(const char* schemaFilename, const char* /*filename*/)
	{
		cfg::TmlParser schemaParser(schemaFilename);
		cfg::NameValuePair schemaCvp;
		if (!schemaParser.getAsTree(schemaCvp, true, true)) {
			std::cerr << "parse " << schemaFilename << " failed" << std::endl;
			std::cerr << "error: " << schemaParser.getExtendedErrorMsg() << std::endl;
			return 1;
		}
		cfg::Value& value = schemaCvp.mValue;
		std::cout << "==================== original ====================" << std::endl;
		std::string s = cfg::tmlstring::valueToString(0, value);
		std::cout << s << std::endl;

		std::cout << "==================== load schema ====================" << std::endl;
		std::string errMsg;
		cfg::NVFragmentSchema nvfs;
		bool rv = cfg::schema::getSchemaFromCfgValue(nvfs, value, errMsg);
		std::cout << "rv: " << (rv ? "true" : "false") << ", err msg: " << errMsg << std::endl;

		std::cout << "==================== schema as tml ====================" << std::endl;
		cfg::Value cfgSchema;
		errMsg.clear();
		rv = cfg::schema::getSchemaAsCfgValue(cfgSchema, nvfs, errMsg);
		std::cout << "rv: " << (rv ? "true" : "false") << ", err msg: " << errMsg << std::endl;
		std::cout << cfg::tmlstring::valueToString(0, cfgSchema) << std::endl;

		//bool rv = cfg::schema::validate(value, errMsg);
		return 0;
	}
}

#ifdef INCLUDE_UNIT_TESTS
static int unitTests();
#endif

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
	if (command == "load-tml") {
		if (argc != 3) {
			std::cerr << "load-tml command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return onlyLoadTml(argv[2], true, true);
	}
	if (command == "load-btml") {
		if (argc != 3) {
			std::cerr << "load-btml command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return onlyLoadBtml(argv[2]);
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
	if (command == "include" || command == "include-buf" ||
			command == "include-once" || command == "include-once-buf") {
		if (argc != 3) {
			std::cerr << "include command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return includeAndPrint(argv[2],
				command == "include-once" || command == "include-once-buf", true, true,
				command == "include-buf" || command == "include-once-buf", false);
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
	if (command == "tml2btml") {
		if (argc != 5) {
			std::cerr << "tml2btml command need exactly 3 arguments" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		bool allFeatures = false;
		bool includeEmptyAndComment = true;
		bool shrinkBtml = false;
		std::string mode = argv[2];
		if (mode == "all") {
			// already the correct values
		}
		else if (mode == "shrink") {
			shrinkBtml = true;
		}
		else if (mode == "strip") {
			includeEmptyAndComment = false;
		}
		else if (mode == "strip-shrink") {
			includeEmptyAndComment = false;
			shrinkBtml = true;
		}
		else if (mode == "afss") { // all-features-strip-shrink
			allFeatures = true;
			includeEmptyAndComment = false;
			shrinkBtml = true;
		}
		else {
			std::cerr << "tml2btml: mode '" << mode << "' is not supported for tml2btml" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		const char* inTmlFilename = argv[3];
		const char* outBtmlFilename = argv[4];
		return convertTmlToBtml(inTmlFilename, outBtmlFilename,
				allFeatures, includeEmptyAndComment, shrinkBtml);
	}
	if (command == "btml2tml") {
		if (argc != 5) {
			std::cerr << "btml2tml command need exactly 3 arguments" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		//bool includeEmptyAndComment = true;
		std::string mode = argv[2];
		if (mode == "all") {
			// already the correct values
		}
		//else if (mode == "strip") {
		//	includeEmptyAndComment = false;
		//}
		else {
			std::cerr << "btml2tml: mode '" << mode << "' is not supported for tml2btml" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		const char* inBtmlFilename = argv[3];
		const char* outTmlFilename = argv[4];
		return convertBtml2Tml(inBtmlFilename, outTmlFilename);
	}
	if (command == "printcppexample") {
		std::string s = cfg::cppstring::valueToString(0, example, false);
		std::cout << s << std::endl;
		return 0;
	}
	if (command == "interpreter-tests") {
		cfg::interpreter::unitTests();
		return 0;
	}
	if (command == "interpret") {
		if (argc != 3) {
			std::cerr << "interpret command need exactly one argument/filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return interpret(argv[2]);
	}
	if (command == "all-features") {
		if (argc != 3 && argc != 4) {
			std::cerr << "all-features command a filename and optional output-filename" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return allFeatures(argv[2], argc == 3, argc >= 4 ? argv[3] : nullptr);
	}
	if (command == "validate") {
		if (argc != 4) {
			std::cerr << "validate command need exactly two arguments (schema-filename and tml-filename)" << std::endl;
			printHelp(argv[0]);
			return 1;
		}
		return validate(argv[2], argv[3]);
	}
	if (command == "unit-tests") {
		return unitTests();
	}
	std::cerr << "command '" << command << "' is not supported" << std::endl;
	printHelp(argv[0]);
	return 1;
}

#ifdef INCLUDE_UNIT_TESTS

static bool testBtmlWithTml(const std::string& tml)
{
	cfg::Value val;
	std::string errMsg;
	if (!cfg::tmlparser::getValueFromString(val, tml, true, true, &errMsg)) {
		std::cout << "'" << tml << "' FAIL: Can't get cfg::Value. err: " << errMsg << std::endl;
		return false;
	}
	std::vector<uint8_t> btml;
	//unsigned int btmlLen = cfg::btmlstream::valueToStream(val, btml);
	unsigned int btmlLen = cfg::btmlstream::valueToStreamWithHeader(val, btml, false);
	if (!btmlLen) {
		std::cout << "'" << tml << "' FAIL: Can't create btml." << std::endl;
		return false;
	}
	if (btmlLen != btml.size()) {
		std::cout << "'" << tml << "' FAIL: btml has wrong size. (" <<
				btmlLen << " != " << btml.size() << ")" << std::endl;
		return false;
	}
	val.clear();
	errMsg.clear();
	bool stringTableExist = false;
	unsigned int stringTableEntryCount = 0;
	unsigned int stringTableSize = 0;
	unsigned int btmlRv = cfg::btmlstream::streamToValueWithHeader(btml.data(),
			static_cast<unsigned int>(btml.size()), val, &errMsg,
			stringTableExist, stringTableEntryCount, stringTableSize);
	if (!btmlRv) {
		std::cout << "'" << tml << "' FAIL: Can't convert back from btml to tml. err: " <<
				errMsg << std::endl;
		return false;
	}
	if (btmlRv != btml.size()) {
		std::cout << "'" << tml << "' FAIL: Not all bytes are used from btml to tml. (" <<
				btmlRv << " != " << btml.size() << "), err: " << errMsg << std::endl;
		//std::cout << "'" << cfg::cfgstring::valueToString(0, val) << "'" << std::endl;
		return false;
	}
	std::string tmlResult = cfg::tmlstring::valueToString(0, val);
	if ((tml.empty() || tml.back() != '\n') && !tmlResult.empty() && tmlResult.back() == '\n') {
		tmlResult.pop_back();
	}
	if (tml != tmlResult) {
		std::cout << "'" << tml << "' FAIL: != '" << tmlResult << "'" << std::endl;
		return false;
	}
	std::cout << "'" << tml << "' OK" << std::endl;
	return true;
}

// return 0 for success, 1 for fail
static int testBtml()
{
	bool success = true;
	std::cout << "*** test btml ***" << std::endl;
	success = testBtmlWithTml("") && success;
	success = testBtmlWithTml("# comment") && success;
	success = testBtmlWithTml("null") && success;
	success = testBtmlWithTml("true") && success;
	success = testBtmlWithTml("false") && success;
	success = testBtmlWithTml("0.123") && success;
	success = testBtmlWithTml("7") && success;
	success = testBtmlWithTml("text") && success;
	success = testBtmlWithTml("0 1 2 3 4 5") && success;
	success = testBtmlWithTml("object\n\ta = 1\n\tb = 2") && success;
	success = testBtmlWithTml("a = b") && success;
	success = testBtmlWithTml("null = true") && success;
	success = testBtmlWithTml("true = null") && success;
	success = testBtmlWithTml("false = 0.123") && success;
	success = testBtmlWithTml("7 = text") && success;
	success = testBtmlWithTml("text = 0 1 2 3 4 5") && success;
	success = testBtmlWithTml("0.1 1.2 3.4 = a b c d e f") && success;
	success = testBtmlWithTml("object\n\ta = 1\n\tb = 2") && success;
	success = testBtmlWithTml("object\n\ta = 1\n\t# a comment\n\tsubobj\n\t\taa = a\n\t\tbb = b\n\tb = 2") && success;
	// return 0 for success, 1 for fail
	return success ? 0 : 1;
}

static int unitTests()
{
	int fail = 0;
	fail = testBtml() || fail;
	return fail;
}
#endif
