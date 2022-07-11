#ifndef CFG_CFG_CPPSTRING_H
#define CFG_CFG_CPPSTRING_H

#include <cfg/export.h>

#include <string>

namespace cfg
{
	class Value;
	class NameValuePair;

	namespace cppstring
	{
		CFG_API
		std::string valueToString(unsigned int deep,
				const Value &cfgValue, bool addFormatArguments, bool addIndention = true);
		CFG_API
		std::string nameValuePairToString(unsigned int deep,
				const NameValuePair &cfgPair, bool addFormatArguments);
	};
}

#endif
