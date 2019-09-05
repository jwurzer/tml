#ifndef TML_CFG_STRING_H
#define TML_CFG_STRING_H

#include <string>

namespace cfg
{
	class Value;
	class NameValuePair;

	namespace cfgstring
	{
		std::string valueToString(unsigned int deep,
				const Value &cfgValue, const std::string &name = "");
		std::string nameValuePairToString(unsigned int deep,
				const NameValuePair &cfgPair, const std::string &name = "");
	};
}

#endif
