#ifndef CFG_CFG_INCLUDE_H
#define CFG_CFG_INCLUDE_H

#include <cfg/export.h>
#include <cfg/cfg.h>
#include <string>
#include <set>

namespace cfg
{
	class FileLoader;

	namespace inc
	{
		CFG_API
		bool loadAndIncludeFiles(Value& outValue, const std::string& filename,
				FileLoader& loader, const std::string& includeKeyword,
				bool inclEmptyLines, bool inclComments,
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
				const std::string& includeKeyword,
				bool inclEmptyLines, bool inclComments,
				std::string& outErrorMsg);

		CFG_API
		bool includeFiles(Value& cfgValue, FileLoader& loader,
				const std::string& includeKeyword,
				bool inclEmptyLines, bool inclComments,
				std::string& outErrorMsg,
				std::set<std::string>& currentIncludedFiles,
				int currentDeep);
	}
}

#endif
