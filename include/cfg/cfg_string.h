#ifndef CFG_CFG_STRING_H
#define CFG_CFG_STRING_H

#include <cfg/export.h>

#include <string>

namespace cfg
{
	class Value;
	class NameValuePair;

	namespace cfgstring
	{
		CFG_API
		std::string valueToString(unsigned int deep,
				const Value &cfgValue, const std::string &name = "");
		CFG_API
		std::string nameValuePairToString(unsigned int deep,
				const NameValuePair &cfgPair, const std::string &name = "");
	};
}

#endif
