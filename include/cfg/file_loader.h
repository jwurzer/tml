#ifndef CFG_FILE_LOADER_H
#define CFG_FILE_LOADER_H

#include <cfg/export.h>
#include <cfg/cfg.h>

namespace cfg
{
	class CFG_API FileLoader
	{
	public:
		virtual std::string getFullFilename(const std::string& includeFilename) const = 0;
		virtual bool loadAndPush(Value& outValue,
				std::string& outFullFilename,
				const std::string& includeFilename,
				bool inclEmptyLines, bool inclComments,
				std::string& outErrorMsg) = 0;
		virtual bool pop() = 0;
		virtual unsigned int getNestedDeep() const = 0;
	};
}

#endif
