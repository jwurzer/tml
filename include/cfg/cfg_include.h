#ifndef CFG_CFG_INCLUDE_H
#define CFG_CFG_INCLUDE_H

#include <cfg/export.h>
#include <cfg/cfg.h>
#include <string>
#include <map>

namespace cfg
{
	class FileLoader;

	namespace inc
	{
		typedef std::map<std::string, unsigned int> TFileMap;

		/**
		 * @param includeKeyword A keyword for include (e.g. "include" or maybe "include-once")
		 *        If it has a include once behaviour depends on the includeOnce
		 *        parameter and not on the include keyword.
		 * @param includeOnce true if a file can only be included once.
		 *        false if a file can be included multiple times.
		 *        If both case should be possible then two different include
		 *        keywords must be used (e.g. "include" and "include-once") and
		 *        this function must be called twice with true and false for this parameter.
		 */
		CFG_API
		bool loadAndIncludeFiles(Value& outValue, TFileMap& outIncludedFiles,
				const std::string& filename, FileLoader& loader,
				const std::string& includeKeyword, bool includeOnce,
				bool inclEmptyLines, bool inclComments, bool withFileBuffering,
				std::string& outErrorMsg);
		/**
		 * Include all files which are specified in cfgValue.
		 * @param cfgValue
		 * @param loader
		 * @param includeKeyword
		 * @return
		 */
		CFG_API
		bool includeFiles(Value& cfgValue, FileLoader& loader,
				const std::string& includeKeyword, bool includeOnce,
				bool inclEmptyLines, bool inclComments, bool withFileBuffering,
				std::string& outErrorMsg);

		CFG_API
		bool includeFiles(Value& cfgValue, FileLoader& loader,
				const std::string& includeKeyword, bool includeOnce,
				bool inclEmptyLines, bool inclComments, bool withFileBuffering,
				std::string& outErrorMsg, TFileMap& currentIncludedFiles,
				int currentDeep);
	}
}

#endif
